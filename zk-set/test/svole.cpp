#include "emp-ot/emp-ot.h"
#include "emp-zk/emp-zk.h"

using namespace emp;
using namespace std;

int port, party;
const int threads = 4;

void check_correctness_svole(block *val, block *mac, int num, BoolIO<NetIO> *io, int party_tmp) {
	if(party_tmp == ALICE) {
		io->send_data(val, num*sizeof(block));
		io->send_data(mac, num*sizeof(block));
        io->flush();
	} else {
		block *vr = new block[num];
		block *mr = new block[num];
		io->recv_data(vr, num*sizeof(block));
		io->recv_data(mr, num*sizeof(block));
		block delta = val[0];
		for(int i = 0; i < num; ++i) {
			gfmul(vr[i], delta, &vr[i]);
			vr[i] = vr[i] ^ mac[i];
		}
        if(memcmp(vr, mr, 16*num) != 0) {
            error("wrong correlations\n");
        }
	}
}

void test_base_svole(BoolIO<NetIO> *ios[threads], int party) {
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);

	ZKBoolCircExec<BoolIO<NetIO>> *exec = (ZKBoolCircExec<BoolIO<NetIO>>*)(CircuitExecution::circ_exec);

	auto start = clock_start();
	BaseSVoleF2k<BoolIO<NetIO>> *svole = new BaseSVoleF2k<BoolIO<NetIO>>(party, ios, exec->ostriple->ferret);

	int test_n = 1024*1024;
	block *val = nullptr, *mac = nullptr;
	if(party == ALICE) val = new block[test_n];
	mac = new block[test_n];
	svole->extend(val, mac, test_n);
	double tt = time_from(start);

	sync_zk_bool<BoolIO<NetIO>>();
	if(party == ALICE) check_correctness_svole(val, mac, test_n, ios[0], party);
	else check_correctness_svole(&(exec->ostriple->delta), mac, test_n, ios[0], party);

	delete svole;

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
    if(cheated != 0) error("zk check fails: prover cheating\n");
	std::cout << "average time: " << tt/test_n << " us" << std::endl;
}

void check_correctness_spfss(block *buf, block delta, int num, int alpha, BoolIO<NetIO> *io) {
	if(party == ALICE) {
		io->send_data(buf, num*sizeof(block));
		io->send_data(&delta, sizeof(block));
        io->flush();
	} else {
		block *vr = new block[num];
		block dr;
		io->recv_data(vr, num*sizeof(block));
		io->recv_data(&dr, sizeof(block));
		for(int i = 0; i < num; ++i) {
            if(i == alpha) {
                gfmul(dr, delta, &dr);
                vr[i] ^= dr;
            } else if(memcmp(vr+i, buf+i, 16) != 0) {
				std::cout << i << std::endl;
				abort();
			}
		}
	}
}

void test_spfss(BoolIO<NetIO> *ios[threads], int party) {
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, 3-party);

	ZKBoolCircExec<BoolIO<NetIO>> *exec = (ZKBoolCircExec<BoolIO<NetIO>>*)(CircuitExecution::circ_exec);

	BaseSVoleF2k<BoolIO<NetIO>> *svole = new BaseSVoleF2k<BoolIO<NetIO>>(3-party, ios, exec->ostriple->ferret);

	int depth = 8;
	int leave_n = 1<<(depth-1);
    int batch_n = 128;
	block *buffer = new block[batch_n*leave_n];
    block *pre_correlation = new block[batch_n];
    block *pre_correlation_x = new block[batch_n];

	auto start = clock_start();

	BaseCot<BoolIO<NetIO>> *base_cot = new BaseCot<BoolIO<NetIO>>(party, ios[0], true);
	OTPre<BoolIO<NetIO>> pre_ot(ios[0], depth-1, batch_n);

	if(party == ALICE) {
        block delta = exec->ostriple->delta;
        base_cot->cot_gen_pre(delta);
        base_cot->cot_gen(&pre_ot, pre_ot.n);
		//block gamma = zero_block;
	    svole->extend(pre_correlation_x, pre_correlation, batch_n);
        for(int i = 0; i < batch_n; ++i) {
            SpfssF2kSend<BoolIO<NetIO>> spfss(ios[0], depth);
            pre_ot.choices_sender();
            spfss.compute(buffer+i*leave_n, delta, pre_correlation[i]);
            spfss.send(&pre_ot, ios[0], i);
            ios[0]->flush();

            check_correctness_spfss(buffer+i*leave_n, delta, leave_n, 0, ios[0]);
        }
	} else {
		//block delta2 = zero_block;
        base_cot->cot_gen_pre();
        base_cot->cot_gen(&pre_ot, pre_ot.n);
	    svole->extend(pre_correlation_x, pre_correlation, batch_n);
        for(int i = 0; i < batch_n; ++i) {
            SpfssF2kRecv<BoolIO<NetIO>> spfss(ios[0], depth);
            pre_ot.choices_recver(spfss.b);
            spfss.get_index();
            spfss.recv(&pre_ot, ios[0], i);
            spfss.compute(buffer+i*leave_n, pre_correlation[i]);
            int alpha = spfss.choice_pos;

            check_correctness_spfss(buffer+i*leave_n, pre_correlation_x[i], leave_n, alpha, ios[0]);
        }
	}

	double tt = time_from(start);

	delete[] buffer;
    delete[] pre_correlation;
    delete[] pre_correlation_x;
    delete svole;
	delete base_cot;

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
    if(cheated != 0) error("zk check fails: prover cheating\n");
	std::cout << "time: " << tt/1000 << " ms" << std::endl;
}

void check_correctness_mpfss_send(block *y, block delta, int num, BoolIO<NetIO> *io) {
	io->send_data(y, num*sizeof(block));
	io->send_data(&delta, sizeof(block));
}

void check_correctness_mpfss_recv(block *y, block *x, vector<uint32_t> &pos, int t, int leave_n, BoolIO<NetIO> *io) {
	block *vr = new block[t*leave_n];
	block delta;
	io->recv_data(vr, t*leave_n*sizeof(block));
	io->recv_data(&delta, sizeof(block));
	int cnt = 0;
	for(int i = 0; i < t; ++i) {
		for(int j = 0; j < leave_n; ++j) {
			if(j == pos[i]) {
				block tmp;
				gfmul(x[cnt], delta, &tmp);
				tmp ^= y[cnt];
				if(memcmp(vr+cnt, &tmp, 16) != 0) {
					std::cout << "choice pos incorrect" << std::endl;
				}
			} else {
				if(memcmp(vr+cnt, y+cnt, 16) != 0) {
					std::cout << "non choice pos incorrect" << std::endl;
				}	
			}
			cnt++;
		}
	}
	delete[] vr;
}

void test_mpfss(BoolIO<NetIO> *ios[threads], int party) {
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, 3-party);
	ZKBoolCircExec<BoolIO<NetIO>> *exec = (ZKBoolCircExec<BoolIO<NetIO>>*)(CircuitExecution::circ_exec);

	int t = 1024;
	int depth = 12;
	int log_bin_sz = depth - 1;
	int leave_n = 1 << log_bin_sz;
	int n = leave_n * t;
	block *buffer = new block[n];
	block *buffer_x = nullptr;
	if(party == BOB) {
		buffer_x = new block[n];
		memset(buffer_x, 0, n*sizeof(block));
	}

	auto start = clock_start();

	ThreadPool pool(threads);
	BaseSVoleF2k<BoolIO<NetIO>> base_svole(3-party, ios, exec->ostriple->ferret);
	BaseCot<BoolIO<NetIO>> *base_cot = new BaseCot<BoolIO<NetIO>>(party, ios[0], true);
	OTPre<BoolIO<NetIO>> pre_ot(ios[0], log_bin_sz, t);
	MpfssRegF2k<BoolIO<NetIO>> mpfss(party, threads, n, t, log_bin_sz, &pool, ios);
	mpfss.set_malicious();
	block *triple_yz = new block[t+1];
	block *triple_x = nullptr;
	memset(triple_yz, 0, (t+1)*sizeof(block));
	if(party == BOB) {
		triple_x = new block[t+1];
		memset(triple_x, 0, (t+1)*sizeof(block));
	}
	base_svole.extend(triple_x, triple_yz, t+1);

	if(party == ALICE) {
		block delta = exec->ostriple->delta;
		base_cot->cot_gen_pre(delta);
		base_cot->cot_gen(&pre_ot, pre_ot.n);

		mpfss.sender_init(delta);
		mpfss.mpfss(&pre_ot, triple_yz, buffer);

		check_correctness_mpfss_send(buffer, delta, n, ios[0]);
	} else {
		base_cot->cot_gen_pre();
		base_cot->cot_gen(&pre_ot, pre_ot.n);

		mpfss.recver_init();
		mpfss.mpfss(&pre_ot, triple_x, triple_yz, buffer);
		mpfss.set_vec_x(buffer_x);

		check_correctness_mpfss_recv(buffer, buffer_x, mpfss.item_pos_recver, t, leave_n, ios[0]);
	}

	double tt = time_from(start);

	delete[] buffer;
	delete base_cot;
	delete[] triple_yz;
    if(party == BOB) {
        delete[] buffer_x;
        delete[] triple_x;
    }

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
    if(cheated != 0) error("zk check fails: prover cheating\n");
	std::cout << "time: " << tt/1000 << " ms" << std::endl;
}

void test_svole(BoolIO<NetIO> *ios[threads], int party) {
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, 3-party);

	ZKBoolCircExec<BoolIO<NetIO>> *exec = (ZKBoolCircExec<BoolIO<NetIO>>*)(CircuitExecution::circ_exec);

	auto start = clock_start();
	SVoleF2k<BoolIO<NetIO>> *svole = new SVoleF2k<BoolIO<NetIO>>(3-party, threads, ios, exec->ostriple->ferret);
	if(party == BOB) svole->setup();
	else svole->setup(exec->ostriple->delta);

	block *val = nullptr, *mac = nullptr;
	uint64_t test_n = 0;
	uint64_t mem_need = svole->byte_memory_need_inplace(test_n);
	if(party == BOB) val = new block[mem_need];
	mac = new block[mem_need];

	svole->extend_inplace(val, mac, mem_need);
	double tt = time_from(start);

	sync_zk_bool<BoolIO<NetIO>>();
	if(party == BOB) check_correctness_svole(val, mac, mem_need, ios[0], ALICE);
	else check_correctness_svole(&(exec->ostriple->delta), mac, mem_need, ios[0], BOB);
    

	delete svole;

	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
    if(cheated != 0) error("zk check fails: prover cheating\n");
	std::cout << "time: " << tt*1000/test_n << " ns/correlation" << std::endl;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(
                new NetIO(party == ALICE?nullptr:"127.0.0.1",port),
                party==ALICE);

    for(int i = 0; i < 10; ++i)
	    test_base_svole(ios, party);
    for(int i = 0; i < 10; ++i)
        test_spfss(ios, party);
    for(int i = 0; i < 10; ++i)
        test_mpfss(ios, party);
    for(int i = 0; i < 10; ++i)
        test_svole(ios, party);

	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
