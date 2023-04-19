#!/bin/bash

# install dependencies
sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y cmake
sudo apt-get install -y git
sudo apt-get install -y libssl-dev
sudo apt-get install -y sudo 
sudo apt-get install -y wget 
sudo apt-get install -y maven 
sudo apt-get install -y default-jdk 
sudo apt-get install -y libgflags-dev 
sudo apt-get install -y libgmp-dev 
sudo apt-get install -y libpq-dev
sudo apt-get install -y libboost-all-dev
sudo apt-get install -y postgresql
sudo apt-get install -y postgresql-contrib
sudo apt-get install -y postgresql-client
sudo apt-get install -y python
sudo apt-get install -y openssh-server
sudo apt-get install -y emacs
sudo apt-get install -y libprotobuf-dev
sudo apt-get install -y protobuf-c-compiler
sudo apt-get install -y libqt5webkit5
sudo apt-get install -y tmux
sudo apt-get install -y valgrind
sudo apt-get install -y dos2unix

# initialize postgres if you install it at first time
sudo /etc/init.d/postgresql start
sudo -i -u postgres
createuser -l -s root
exit

#create vaultdb role, can use lower privs if preferred
sudo psql postgres -c "CREATE ROLE vaultdb WITH login superuser"
createdb vaultdb

# load dbs
# For prover (ALICE):
dropdb -U vaultdb --if-exists tpch_60k
dropdb -U vaultdb --if-exists tpch_120k
dropdb -U vaultdb --if-exists tpch_240k

createdb -U vaultdb tpch_60k
createdb -U vaultdb tpch_120k
createdb -U vaultdb tpch_240k
psql -U vaultdb tpch_60k < dbs/tpch_60k.sql
psql -U vaultdb tpch_120k < dbs/tpch_120k.sql
psql -U vaultdb tpch_240k < dbs/tpch_240k.sql

# For verifier (BOB):
dropdb -U vaultdb --if-exists tpch_zk_bob
createdb -U vaultdb tpch_zk_bob
psql -U vaultdb tpch_zk_bob < dbs/tpch_zk_bob.sql

# install emp toolkits
rm -rf emp-* install.py

wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py
python3 install.py --deps --tool --ot --sh2pc --zk

# install zk set operations
cd zk-set
cmake .
make
sudo make install

# install pqxx
cd ..
git clone --branch 6.2.5 https://github.com/jtv/libpqxx.git
cd libpqxx
./configure  --disable-documentation --enable-shared
make 
sudo make install
cd ..

#end
echo "Setup completed successfully."
