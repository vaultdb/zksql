// source: https://github.com/olt/libpq/blob/master/oid/types.go

#ifndef PQ_OID_DEFS_H
#define PQ_OID_DEFS_H



#define OID_BOOL 16
#define OID_BYTEA 17
#define OID_CHAR 18
#define OID_NAME 19
#define OID_INT8 20
#define OID_INT2 21
#define OID_INT2VECTOR 22
#define OID_INT4 23
#define OID_REGPROC 24
#define OID_TEXT 25
#define OID_OID 26
#define OID_TID 27
#define OID_XID 28
#define OID_CID 29
#define OID_OIDVECTOR 30
#define OID_PG_TYPE 71
#define OID_PG_ATTRIBUTE 75
#define OID_PG_PROC 81
#define OID_PG_CLASS 83
#define OID_JSON 114
#define OID_XML 142
#define OID__XML 143
#define OID_PG_NODE_TREE 194
#define OID__JSON 199
#define OID_SMGR 210
#define OID_POINT 600
#define OID_LSEG 601
#define OID_PATH 602
#define OID_BOX 603
#define OID_POLYGON 604
#define OID_LINE 628
#define OID__LINE 629
#define OID_CIDR 650
#define OID__CIDR 651
#define OID_FLOAT4 700
#define OID_FLOAT8 701
#define OID_ABSTIME 702
#define OID_RELTIME 703
#define OID_TINTERVAL 704
#define OID_UNKNOWN 705
#define OID_CIRCLE 718
#define OID__CIRCLE 719
#define OID_MONEY 790
#define OID__MONEY 791
#define OID_MACADDR 829
#define OID_INET 869
#define OID__BOOL 1000
#define OID__BYTEA 1001
#define OID__CHAR 1002
#define OID__NAME 1003
#define OID__INT2 1005
#define OID__INT2VECTOR 1006
#define OID__INT4 1007
#define OID__REGPROC 1008
#define OID__TEXT 1009
#define OID__TID 1010
#define OID__XID 1011
#define OID__CID 1012
#define OID__OIDVECTOR 1013
#define OID__BPCHAR 1014
#define OID__VARCHAR 1015
#define OID__INT8 1016
#define OID__POINT 1017
#define OID__LSEG 1018
#define OID__PATH 1019
#define OID__BOX 1020
#define OID__FLOAT4 1021
#define OID__FLOAT8 1022
#define OID__ABSTIME 1023
#define OID__RELTIME 1024
#define OID__TINTERVAL 1025
#define OID__POLYGON 1027
#define OID__OID 1028
#define OID_ACLITEM 1033
#define OID__ACLITEM 1034
#define OID__MACADDR 1040
#define OID__INET 1041
#define OID_BPCHAR 1042
#define OID_VARCHAR 1043
#define OID_DATE 1082
#define OID_TIME 1083
#define OID_TIMESTAMP 1114
#define OID__TIMESTAMP 1115
#define OID__DATE 1182
#define OID__TIME 1183
#define OID_TIMESTAMPTZ 1184
#define OID__TIMESTAMPTZ 1185
#define OID_INTERVAL 1186
#define OID__INTERVAL 1187
#define OID__NUMERIC 1231
#define OID_PG_DATABASE 1248
#define OID__CSTRING 1263
#define OID_TIMETZ 1266
#define OID__TIMETZ 1270
#define OID_BIT 1560
#define OID__BIT 1561
#define OID_VARBIT 1562
#define OID__VARBIT 1563
#define OID_NUMERIC 1700
#define OID_REFCURSOR 1790
#define OID__REFCURSOR 2201
#define OID_REGPROCEDURE 2202
#define OID_REGOPER 2203
#define OID_REGOPERATOR 2204
#define OID_REGCLASS 2205
#define OID_REGTYPE 2206
#define OID__REGPROCEDURE 2207
#define OID__REGOPER 2208
#define OID__REGOPERATOR 2209
#define OID__REGCLASS 2210
#define OID__REGTYPE 2211
#define OID_RECORD 2249
#define OID_CSTRING 2275
#define OID_ANY 2276
#define OID_ANYARRAY 2277
#define OID_VOID 2278
#define OID_TRIGGER 2279
#define OID_LANGUAGE_HANDLER 2280
#define OID_INTERNAL 2281
#define OID_OPAQUE 2282
#define OID_ANYELEMENT 2283
#define OID__RECORD 2287
#define OID_ANYNONARRAY 2776
#define OID_PG_AUTHID 2842
#define OID_PG_AUTH_MEMBERS 2843
#define OID__TXID_SNAPSHOT 2949
#define OID_UUID 2950
#define OID__UUID 2951
#define OID_TXID_SNAPSHOT 2970
#define OID_FDW_HANDLER 3115
#define OID_ANYENUM 3500
#define OID_TSVECTOR 3614
#define OID_TSQUERY 3615
#define OID_GTSVECTOR 3642
#define OID__TSVECTOR 3643
#define OID__GTSVECTOR 3644
#define OID__TSQUERY 3645
#define OID_REGCONFIG 3734
#define OID__REGCONFIG 3735
#define OID_REGDICTIONARY 3769
#define OID__REGDICTIONARY 3770
#define OID_ANYRANGE 3831
#define OID_EVENT_TRIGGER 3838
#define OID_INT4RANGE 3904
#define OID__INT4RANGE 3905
#define OID_NUMRANGE 3906
#define OID__NUMRANGE 3907
#define OID_TSRANGE 3908
#define OID__TSRANGE 3909
#define OID_TSTZRANGE 3910
#define OID__TSTZRANGE 3911
#define OID_DATERANGE 3912
#define OID__DATERANGE 3913
#define OID_INT8RANGE 3926
#define OID__INT8RANGE 3927



static FieldType getFieldTypeFromOid(pqxx::oid oid) {
    switch (oid) {
        case OID_BPCHAR:
        case OID_VARCHAR:
            return FieldType::STRING;
        case OID_INT4:
            return FieldType::INT;
        case OID_INT8:
            return FieldType::LONG;
        case OID__NUMERIC:
        case OID_NUMERIC:
        case OID_FLOAT4:
            return FieldType::FLOAT;
        case OID_DATE:
        case OID__DATE:
            return FieldType::DATE;
        case OID_BOOL:
        case OID__BOOL:
            return FieldType::BOOL;
        default: {
            throw std::invalid_argument("Unsupported column type " + std::to_string(oid));
        }
    }
}
#endif //PQ_OID_DEFS_H
