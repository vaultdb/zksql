# Docker swarm set up for two instances on AWS

## Setup
### On each aws instance
#### apt update and install docker
`sudo apt-get update && sudo apt-get upgrade -y`

`sudo apt install docker.io`
#### Grant permission for ports
`sudo ufw allow 2377/tcp`

`sudo ufw allow 7946/tcp`

`sudo ufw allow 7946/udp`

`sudo ufw allow 4789/udp`

#### Pull docker image
`sudo docker pull jennierogers/vaultdb-deployment:zksql`
### On Alice
#### Use Alice instance as docker swarm manager
`sudo docker swarm init`
### On Bob 
#### Use Bob instance as docker swarm worker
`sudo docker swarm join --token … alice_ip:2377`
### On Alice
#### Set up overlay network named vaultdb-net
`sudo docker network create -d overlay --subnet=126.137.1.0/24 --attachable vaultdb-net`
### On Bob
#### Create container
`sudo docker run -itd --network vaultdb-net --ip 126.137.1.20 --name vaultdb-container jennierogers/vaultdb-deployment:zksql`
### On Alice
#### Create container
`sudo docker run -itd --network vaultdb-net --ip 126.137.1.10 --name vaultdb-container jennierogers/vaultdb-deployment:zksql`
### On each aws instance
#### Enter the container
`sudo docker exec -u vaultdb -it vaultdb-container /bin/bash`
#### Put zksql directory to /home/vaultdb/
#### Install ZK set
`cd /home/vaultdb/zksql/emp-zk-set`

`cmake .`

`make`

`sudo make install`
#### Load dbs
`cd /home/vaultdb/zksql/dbs`

#### For Alice:
`createdb tpch_60k`

`createdb tpch_120k`

`createdb tpch_240k`

`psql tpch_60k < tpch_60k.sql`

`psql tpch_120k < tpch_120k.sql`

`psql tpch_240k < tpch_240k.sql`

#### For Bob:
`createdb tpch_zk_bob`

`psql tpch_zk_bob < tpch_zk_bob.sql`

#### Confirm database in use for each container
#### tpch_60k/tpch_120k/tpch_240k for Alice and tpch_zk_bob for Bob
`vim /home/vaultdb/zksql/src/main/cpp/test/support/zk_base_test.h`

#### cmake in release mode and make tpch_test
`cd /home/vaultdb/zksql/src/main/cpp`

`cmake -DCMAKE_BUILD_TYPE=Release .`

`make clean`

`make tpch_test`
### On Alice container
#### Run tpch_test
`nohup ./bin/tpch_test --party=1 --alice_host="126.137.1.10" > alice.txt &`
### On Bob container
#### Run tpch_test
`nohup ./bin/tpch_test --party=2 --alice_host="126.137.1.10" > bob.txt &`

### Change test db
#### tpch_60k/tpch_120k/tpch_240k for Alice and tpch_zk_bob for Bob
`vim /home/vaultdb/zksql/src/main/cpp/test/support/zk_base_test.h`

## Reference
[official doc](https://docs.docker.com/network/network-tutorial-overlay/#use-an-overlay-network-for-standalone-containers),
[tech blog](https://medium.com/@tukai.anirban/container-networking-overlay-networks-b712d6ddfb67)
