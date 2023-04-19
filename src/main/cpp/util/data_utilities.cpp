#include <cstddef>
#include <assert.h>
#include <data/psql_data_provider.h>
#include <sys/stat.h>
#include <union.h>
#include "data_utilities.h"


using namespace vaultdb;

unsigned char DataUtilities::reverse(unsigned char b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}







// in some cases, like with LIMIT, we can't just run over tpch_unioned
std::shared_ptr<PlainTable>
DataUtilities::getUnionedResults(const std::string &aliceDb, const std::string &bobDb, const std::string &sql,
                                 const bool &hasDummyTag, const size_t & limit) {

    PsqlDataProvider dataProvider;

    std::shared_ptr<PlainTable> lhs = dataProvider.getQueryTable(aliceDb, sql,
                                                                 hasDummyTag); // dummyTag true not yet implemented
    std::shared_ptr<PlainTable> rhs = dataProvider.getQueryTable(bobDb, sql, hasDummyTag);

    if(limit > 0) {
        lhs->resize(limit);
        rhs->resize(limit);
    }


    ZkQueryTable lhs_zk(lhs, shared_ptr<SecureTable>(), emp::ALICE, nullptr);
    ZkQueryTable rhs_zk(rhs, shared_ptr<SecureTable>(), emp::ALICE, nullptr);


    Union result(lhs_zk, rhs_zk);
    return result.run().plain_table_;
}



std::shared_ptr<PlainTable > DataUtilities::getQueryResults(const std::string & dbName, const std::string & sql, const bool & hasDummyTag) {
    PsqlDataProvider dataProvider;
    return dataProvider.getQueryTable(dbName, sql, hasDummyTag);
}

std::string DataUtilities::queryDatetime(const string &colName) {
    return "CAST(EXTRACT(epoch FROM " + colName + ") AS BIGINT) " + colName; // last colName for aliasing
}




void DataUtilities::writeFile(const string &fileName, vector<int8_t> contents) {
    std::ofstream outFile(fileName.c_str(), std::ios::out | std::ios::binary);
    if(!outFile.is_open()) {
        throw std::invalid_argument("Could not write output file " + fileName);
    }
    outFile.write((char *) contents.data(), contents.size());
    outFile.close();
}

void DataUtilities::writeFile(const string &fileName, const string &contents) {
    std::ofstream outFile(fileName.c_str(), std::ios::out);
    if(!outFile.is_open()) {
        throw std::invalid_argument("Could not write output file " + fileName);
    }
    outFile.write(contents.c_str(), contents.size());
    outFile.close();
}


// reads binary file
vector<int8_t> DataUtilities::readFile(const std::string & fileName) {
    // read in binary and then xor it with other side to secret share it.
    // get file size
    struct stat fileStats;
    size_t fileSize; // bytes
    if (stat(fileName.c_str(), &fileStats) == 0) {
        fileSize = fileStats.st_size;
    }
    else {
        throw std::invalid_argument("Can't open input file " + fileName + "!");
    }


    std::vector<int8_t> fileBytes;
    fileBytes.resize(fileSize);
    int8_t  *shares = fileBytes.data();
    std::ifstream inputFile(fileName, std::ios::in | std::ios::binary);
    inputFile.read((char *) shares, fileSize);
    inputFile.close();

    return fileBytes;

}

SortDefinition DataUtilities::getDefaultSortDefinition(const uint32_t &colCount) {
    SortDefinition  sortDefinition;

    for(uint32_t i = 0; i < colCount; ++i) {
        sortDefinition.push_back(ColumnSort(i, SortDirection::ASCENDING));
    }

    return sortDefinition;

}

std::shared_ptr<PlainTable > DataUtilities::removeDummies(const std::shared_ptr<PlainTable> &input) {
    // only works for plaintext tables
    assert(!input->isEncrypted());
    int outputTupleCount = input->getTrueTupleCount();

    int writeCursor = 0;
    std::shared_ptr<PlainTable > output(new PlainTable(outputTupleCount, *input->getSchema(), input->getSortOrder()));

    for(size_t i = 0; i < input->getTupleCount(); ++i) {
        PlainTuple tuple = input->getTuple(i);
        if(!tuple.getDummyTag()) {
            output->putTuple(writeCursor, tuple);
            ++writeCursor;
        }
    }

    return output;
}

ZkQueryTable DataUtilities::removeTrailingZeroTuples(const ZkQueryTable &input, const std::shared_ptr<PlainTable> &tableWithoutTrailingZero) {
    ZkQueryTable result;
    int tupleSize = tableWithoutTrailingZero->getTupleCount();

    if(input.party_ == ALICE) {
        shared_ptr<PlainTable> plainTable(new PlainTable(tableWithoutTrailingZero->getTupleCount(), QuerySchema::toPlain(*tableWithoutTrailingZero->getSchema()), tableWithoutTrailingZero->getSortOrder()));
        int plainTablePos = 0;

        shared_ptr<PlainTable> inputPlainTable = input.plain_table_;

        for(size_t i = 0; i < inputPlainTable->getTupleCount(); ++i) {
            PlainTuple curTuple = inputPlainTable->getTuple(i);

            bool allZero = true;

            for(size_t j = 0; j < curTuple.getFieldCount(); ++j) {
                PlainField curField = curTuple.getField(j);

                switch(curField.payload_.which()) {
                    case 0: {
                        bool curFieldBoolValue = curField.getValue<bool>();
                        if(curFieldBoolValue) {
                            allZero = false;
                        }
                        break;
                    }
                    case 1: {
                        int32_t curFieldInt32Value = curField.getValue<int32_t>();
                        if (curFieldInt32Value != 0) {
                            allZero = false;
                        }
                        break;
                    }
                    case 2: {
                        int64_t curFieldInt64Value = curField.getValue<int64_t>();
                        if (curFieldInt64Value != 0) {
                            allZero = false;
                        }
                        break;
                    }
                    case 3: {
                        float_t curFieldFloatValue = curField.getValue<float_t>();
                        if (curFieldFloatValue != 0) {
                            allZero = false;
                        }
                        break;
                    }
                    case 4: {
                        string curFieldStringValue = curField.getValue<string>();
                        if (curFieldStringValue != "") {
                            allZero = false;
                        }
                        break;
                    }
                }
            }

            if(!allZero) {
                plainTable->putTuple(plainTablePos++,inputPlainTable->getTuple(i));
            }
        }

        result = ZkQueryTable(plainTable,input.netio_,tupleSize * input.getSchema()->size(), ALICE);
    }
    else {
        assert(input.party_ == BOB);
        result = ZkQueryTable(*(input.getSchema()),tableWithoutTrailingZero->getSortOrder(),input.netio_,tupleSize * input.getSchema()->size(), BOB);
    }

    return result;
}

std::shared_ptr<PlainTable> DataUtilities::removeTrailingZeroTuples(const std::shared_ptr<PlainTable> &input,
                                                                    const int &tupleSize, const QuerySchema &schema,
                                                                    const SortDefinition &sortDef) {
    shared_ptr<PlainTable> plainTable(new PlainTable(tupleSize, schema, sortDef));
    int plainTablePos = 0;

    for(size_t i = 0; i < input->getTupleCount(); ++i) {
        PlainTuple curTuple = input->getTuple(i);

        bool allZero = true;

        for(size_t j = 0; j < curTuple.getFieldCount(); ++j) {
            PlainField curField = curTuple.getField(j);

            switch(curField.payload_.which()) {
                case 0: {
                    bool curFieldBoolValue = curField.getValue<bool>();
                    if(curFieldBoolValue) {
                        allZero = false;
                    }
                    break;
                }
                case 1: {
                    int32_t curFieldInt32Value = curField.getValue<int32_t>();
                    if (curFieldInt32Value != 0) {
                        allZero = false;
                    }
                    break;
                }
                case 2: {
                    int64_t curFieldInt64Value = curField.getValue<int64_t>();
                    if (curFieldInt64Value != 0) {
                        allZero = false;
                    }
                    break;
                }
                case 3: {
                    float_t curFieldFloatValue = curField.getValue<float_t>();
                    if (curFieldFloatValue != 0) {
                        allZero = false;
                    }
                    break;
                }
                case 4: {
                    string curFieldStringValue = curField.getValue<string>();
                    if (curFieldStringValue != "") {
                        allZero = false;
                    }
                    break;
                }
            }
        }

        if(!allZero) {
            plainTable->putTuple(plainTablePos++,input->getTuple(i));
        }
    }

    return plainTable;
}

ZkQueryTable DataUtilities::padZeros(const ZkQueryTable &table, int padSize) {
    ZkQueryTable result;
    int resultSize = table.getTupleCount() + padSize;

    if(table.party_ == ALICE) {
        shared_ptr<PlainTable> plainTable = table.plain_table_;
        shared_ptr<PlainTable> paddedTable(new PlainTable(resultSize, *plainTable->getSchema(),table.getSortOrder()));

        for(int i = 0; i < resultSize; ++i) {
            paddedTable->getTuple(i).setDummyTag(true);
        }

        for(size_t i = 0; i < plainTable->getTupleCount(); ++i) {
            paddedTable->putTuple(i, plainTable->getTuple(i));
        }

        result = ZkQueryTable(paddedTable, table.netio_, resultSize * table.getSchema()->size(),ALICE);
    }
    else {
        assert(table.party_ == BOB);
        result = ZkQueryTable(*(table.getSchema()), table.getSortOrder(),table.netio_,resultSize * table.getSchema()->size(),BOB);
    }

    //result.netio_->flush();

    return result;
}

std::shared_ptr<PlainTable> DataUtilities::zeroOutDummiesAndSort(std::shared_ptr<PlainTable> table) {
    shared_ptr<PlainTable> plainTableOutput(new PlainTable(table->getTupleCount(),*table->getSchema(),table->getSortOrder()));

    for(size_t i = 0; i < plainTableOutput->getTupleCount(); ++i) {
        plainTableOutput->getTuple(i).setDummyTag(true);
    }

    int tablePos = 0;
    for(size_t i = 0; i < table->getTupleCount(); ++i) {
        if(!table->getTuple(i).getDummyTag()) {
            plainTableOutput->putTuple(tablePos++, table->getTuple(i));
        }
    }

    return plainTableOutput;
}

std::shared_ptr<PlainTable >
DataUtilities::getExpectedResults(const string &dbName, const string &sql, const bool &hasDummyTag,
                                  const int &sortColCount) {

    std::shared_ptr<PlainTable > expected = DataUtilities::getQueryResults(dbName, sql, hasDummyTag);
    SortDefinition expectedSortOrder = DataUtilities::getDefaultSortDefinition(sortColCount);
    expected->setSortOrder(expectedSortOrder);
    return expected;
}


std::string DataUtilities::printSortDefinition(const SortDefinition &sortDefinition) {
    std::stringstream  result;
    result << "{";
    bool init = false;
    for(ColumnSort c : sortDefinition) {
        if(init)
            result << ", ";
        string direction = (c.second == SortDirection::ASCENDING) ? "ASC" : "DESC";
        result << "<" << c.first << ", "
               << direction << "> ";

        init = true;
    }

    result << ")";
    return result.str();
}


std::string DataUtilities::printFirstBytes(vector<int8_t> &bytes, const int &byteCount) {
    std::stringstream ss;

    assert(byteCount > 0 && (size_t) byteCount <= bytes.size());

    vector<int8_t>::iterator  readPos = bytes.begin();
    ss << (int) *readPos;
    while((readPos - bytes.begin()) < byteCount) {
        ++readPos;
        ss << "," << (int) *readPos;
    }
    return ss.str();

}


std::string DataUtilities::revealAndPrintFirstBytes(vector<Bit> &bits, const int & byteCount) {
    Integer anInt(bits.size(), 0L);
    anInt.bits = bits;
    std::string bitString = anInt.reveal<std::string>(emp::PUBLIC);

    vector<int8_t> decodedBytes = Utilities::boolsToBytes(bitString);
    return printFirstBytes(decodedBytes, byteCount);

}

size_t DataUtilities::get_tuple_cnt(const string &db_name, const string &sql, bool has_dummy_tag) {
    if(has_dummy_tag)  { // run it and count
        shared_ptr<PlainTable> res = DataUtilities::getQueryResults(db_name, sql, true);
        return res->getTrueTupleCount();
    }

    string query = "SELECT COUNT(*) FROM (" + sql + ") q";
    shared_ptr<PlainTable> res = DataUtilities::getQueryResults(db_name, query, false);
    return res->getTuple(0).getField(0).getValue<int64_t>();

}

vector<string> DataUtilities::readTextFile(const string &filename) {
    std::vector<std::string> lines;
    std::ifstream inFile(filename);
    std::string line;


    if(!inFile)
    {
        string cwd = Utilities::getCurrentWorkingDirectory();
        throw std::invalid_argument("Unable to open file: " + filename + " from " + cwd);
    }


    while (std::getline(inFile, line))
    {
        lines.push_back(line);
    }

    return lines;
}

bool DataUtilities::isOrdinal(const string &s) {
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

// check SecureTable and PlainTable have same contents
void DataUtilities::verifyTable(const ZkQueryTable &table) {
    shared_ptr<PlainTable> r = table.secure_table_->reveal(PUBLIC);

        if(table.party_ == BOB) {
            return;
        }
        if (!(*r == *(table.plain_table_)))
            cout << "gotcha!";

        assert(*r == *(table.plain_table_));

}





