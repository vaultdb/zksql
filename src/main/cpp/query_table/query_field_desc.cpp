#include "query_field_desc.h"
#include "util/type_utilities.h"

using namespace vaultdb;

// default constructor setting up unique_ptr
QueryFieldDesc::QueryFieldDesc() : type_(FieldType::INVALID) {

}

int QueryFieldDesc::getOrdinal() const {
  return QueryFieldDesc::ordinal_;
}


const std::string &QueryFieldDesc::getName() const {
  return QueryFieldDesc::name_;
}

FieldType QueryFieldDesc::getType() const {
    return type_;
}


const std::string &QueryFieldDesc::getTableName() const {
  return QueryFieldDesc::table_name;
}

size_t QueryFieldDesc::size() const {
    FieldType typeId  = getType();
    size_t fieldSize = TypeUtilities::getTypeSize(typeId);
    if(QueryFieldDesc::type_ == FieldType::STRING || QueryFieldDesc::type_ == FieldType::SECURE_STRING )  {
        fieldSize *= string_length_;
    }

    return fieldSize;

}

void QueryFieldDesc::setStringLength(size_t len) {

    string_length_ = len;

}

std::ostream &vaultdb::operator<<(std::ostream &os,  const QueryFieldDesc &desc)  {
    os << "#" << desc.getOrdinal() << " " << TypeUtilities::getTypeString(desc.getType());
    if(desc.getType() == FieldType::STRING || desc.getType() == FieldType::SECURE_STRING) {
        os << "(" << desc.getStringLength() << ")";
    }

    os << " " << desc.getTableName() << "." << desc.getName();
    return os;
}

QueryFieldDesc& QueryFieldDesc::operator=(const QueryFieldDesc& src)  {


    this->type_ = src.type_;
    this->name_ = src.name_;
    this->ordinal_ = src.ordinal_;
    this->table_name = src.table_name;
    this->string_length_ = src.string_length_;

    return *this;
}

// only checking for relation compatibility, so don't care about table name or field name
bool QueryFieldDesc::operator==(const QueryFieldDesc& other) {

    // if types are the same, or int32_t --> date
    if (!(this->getType() == other.getType() ||
          (this->getType() == FieldType::LONG && other.getType() == FieldType::DATE) ||
          (other.getType() == FieldType::LONG && this->getType() == FieldType::DATE))) {
        return false;
    }





    if(other.getOrdinal() != other.getOrdinal()) {

        return false;
    }

    if(other.string_length_ != this->string_length_) {

        return false;
    }

    return true;


}


