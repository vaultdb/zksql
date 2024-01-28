# ZKSQL: Verifiable and Efficient Query Evaluation with Zero-Knowledge Proofs

--------------------------------------------------------------------------------
Authors
--------------------------------------------------------------------------------

Xiling Li, Chenkai Weng, Yongxin Xu, Xiao Wang, Jennie Rogers

{xiling.li@,ckweng@u.@, yongxinxu2022@u.,wangxiao@cs., jennie@}northwestern.edu

--------------------------------------------------------------------------------
Abstract
--------------------------------------------------------------------------------

Individuals and organizations are using databases to store personal information at an unprecedented rate. This creates a quandary for data providers. They are responsible for protecting the privacy of individuals described in their database. On the other hand, data providers are sometimes required to provide statistics about their data instead of sharing it wholesale with strong assurances that these answers are correct and complete such as in regulatory filings for the US SEC and other goverment organizations.  

We introduce a system, ZKSQL, that provides authenticated answers to ad-hoc SQL queries with zero-knowledge proofs. Its proofs show that the answers are correct and sound with respect to the database's contents and they do not divulge any  information about its input records. This system constructs proofs over the steps in a query's evaluation and it accelerates this process with authenticated set operations. We validate the efficiency of this approach over a suite of TPC-H queries and our results show that ZKSQL achieves two orders of magnitude speedup over the baseline.

--------------------------------------------------------------------------------
Dependencies
--------------------------------------------------------------------------------
* PostgreSQL 9.5+
* Apache Calcite 1.8+
* Apache Maven 3+
* Java 8+
* JavaCC 5.0+
* Maven 4+
* Python 2.7+
* cmake 3.11+
* Protobuf (protobuf-c-compiler protobuf-compiler in brew)
* libpqxx 6.2.5
* libgflags-dev
* libgrpc++-dev, libgrpc-dev

--------------------------------------------------------------------------------
Docker 
--------------------------------------------------------------------------------

In this work, we recommend to run the code with docker setup as shown in:
```
docker_setup.md
```
Otherwise, you setup and run the code as shown in the following sections.

--------------------------------------------------------------------------------
Setup
--------------------------------------------------------------------------------

Install the dependencies above as needed

Configure psql and load 60k, 120k and 240k databases

Install zk set operations, emp toolkits (VOLE-based protocols) and pqxx
```
./setup.sh
```
--------------------------------------------------------------------------------
Frontend 
--------------------------------------------------------------------------------

In our experiments, we generate our plans for backend based on queries in TPC-H benchmark and plans can be found:
```
cd src/main/cpp/conf/plans/
```
To cutomize plan, you should use maven to manage all required dependencies:
```
mvn compile
```
Then you can parse a SQL query to its canonicalized query tree (in json):
```
mvn compile exec:java -Dexec.mainClass="org.vaultdb.ParseSqlToJson" -Dexec.args="<db name> <file with SQL query>  <path to write output file>"
```
For example, to prepare a query for the tpch database in PostgreSQL with the query stored in conf/workload/tpch/queries/01.sql writing the query tree to conf/workload/tpch/plans/01.json, run:
```
mvn compile exec:java -Dexec.mainClass="org.vaultdb.ParseSqlToJson" -Dexec.args="tpch   conf/workload/tpch/queries/01.sql  conf/workload/tpch/plans"
```
--------------------------------------------------------------------------------
Backend
--------------------------------------------------------------------------------
Build:
```
cd src/main/cpp
cmake .
```
Confirm database in use:
```
# db_name_ = tpch_60k/tpch_120k/tpch_240k for Alice and db_name_ = tpch_zk_bob for Bob
vim ./test/support/zk_base_test.h
```
Make TPC-H tests:
```
make tpch_test
```
Run tests for Alice (prover) and Bob (verifier) concurrently in separate machines:
```
# Alice machine
sudo chmod +x ./run-alice.sh
./run-alice.sh tpch_test "alice_ip_address"
# Bob machine
sudo chmod +x ./run-bob.sh
./run-bob.sh tpch_test "alice_ip_address"
```
To switch databases in use, please modify:
```
# Create an empty copy of your new DB for Bob (the verifier) with table definitions alone and no rows
# Bob will use this for schema inference
# E.g., db_name_ = "tpch_zk_bob" for Bob
vim ./test/support/zk_base_test.h
make tpch_test
```
--------------------------------------------------------------------------------
References
--------------------------------------------------------------------------------

Xiling Li and Chenkai Weng and Yongxin Xu and Xiao Wang and Jennie Rogers. [ZKSQL: Verifiable and Efficient Query Evaluation with Zero-Knowledge Proofs](https://www.vldb.org/pvldb/vol16/p1804-li.pdf), In Proceedings of the VLDB Endowment (PVLDB), Volume 16, Issue 8, 2023.

Xiao Wang, Alex J. Malozemoff, and Jonathan Katz. EMP-toolkit: Efficient MultiParty computation toolkit. https://github.com/emp-toolkit,  2016. 

Kang Yang, Pratik Sarkar, Chenkai Weng, and Xiao Wang. 2021. QuickSilver: Efficient and Affordable Zero-Knowledge Proofs for Circuits and Polynomials over Any Field. In Proceedings of the 2021 ACM SIGSAC Conference on Computer and Communications Security (CCS '21). Association for Computing Machinery, New York, NY, USA, 2986–3001. https://doi.org/10.1145/3460120.3484556

Nicholas Franzese, Jonathan Katz, Steve Lu, Rafail Ostrovsky, Xiao Wang, and Chenkai Weng. 2021. Constant-Overhead Zero-Knowledge for RAM Programs. In Proceedings of the 2021 ACM SIGSAC Conference on Computer and Communications Security (CCS '21). Association for Computing Machinery, New York, NY, USA, 178–191. https://doi.org/10.1145/3460120.3484800

--------------------------------------------------------------------------------
Acknowledgement
--------------------------------------------------------------------------------

This work is supported in part by DARPA under Contract No.HR001120C0087, NSF awards #2016240, #1846447, #2236819, and research awards from Facebook and Google. The views, opinions, and/or findings expressed are those of the author(s) and should not be interpreted as representing the official views or policies of the Department of Defense or the U.S. Government.
