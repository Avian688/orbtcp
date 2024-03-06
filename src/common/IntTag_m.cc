//
// Generated file, do not edit! Created by opp_msgtool 6.0 from common/IntTag.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "IntTag_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace inet {

class IntDataVecDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    IntDataVecDescriptor();
    virtual ~IntDataVecDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(IntDataVecDescriptor)

IntDataVecDescriptor::IntDataVecDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::IntDataVec)), "")
{
    propertyNames = nullptr;
}

IntDataVecDescriptor::~IntDataVecDescriptor()
{
    delete[] propertyNames;
}

bool IntDataVecDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<IntDataVec *>(obj)!=nullptr;
}

const char **IntDataVecDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = { "existingClass",  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *IntDataVecDescriptor::getProperty(const char *propertyName) const
{
    if (!strcmp(propertyName, "existingClass")) return "";
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int IntDataVecDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int IntDataVecDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *IntDataVecDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int IntDataVecDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *IntDataVecDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **IntDataVecDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *IntDataVecDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int IntDataVecDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void IntDataVecDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'IntDataVec'", field);
    }
}

const char *IntDataVecDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string IntDataVecDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void IntDataVecDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntDataVec'", field);
    }
}

omnetpp::cValue IntDataVecDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'IntDataVec' as cValue -- field index out of range?", field);
    }
}

void IntDataVecDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntDataVec'", field);
    }
}

const char *IntDataVecDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr IntDataVecDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void IntDataVecDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    IntDataVec *pp = omnetpp::fromAnyPtr<IntDataVec>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntDataVec'", field);
    }
}

Register_Class(IntMetaData)

IntMetaData::IntMetaData() : ::omnetpp::cObject()
{
}

IntMetaData::IntMetaData(const IntMetaData& other) : ::omnetpp::cObject(other)
{
    copy(other);
}

IntMetaData::~IntMetaData()
{
}

IntMetaData& IntMetaData::operator=(const IntMetaData& other)
{
    if (this == &other) return *this;
    ::omnetpp::cObject::operator=(other);
    copy(other);
    return *this;
}

void IntMetaData::copy(const IntMetaData& other)
{
    this->hopName = other.hopName;
    this->ts = other.ts;
    this->qLen = other.qLen;
    this->txBytes = other.txBytes;
    this->b = other.b;
    this->averageRtt = other.averageRtt;
    this->numOfFlows = other.numOfFlows;
    this->rxQlen = other.rxQlen;
    this->numOfFlowsInInitialPhase = other.numOfFlowsInInitialPhase;
}

void IntMetaData::parsimPack(omnetpp::cCommBuffer *b) const
{
    doParsimPacking(b,this->hopName);
    doParsimPacking(b,this->ts);
    doParsimPacking(b,this->qLen);
    doParsimPacking(b,this->txBytes);
    doParsimPacking(b,this->b);
    doParsimPacking(b,this->averageRtt);
    doParsimPacking(b,this->numOfFlows);
    doParsimPacking(b,this->rxQlen);
    doParsimPacking(b,this->numOfFlowsInInitialPhase);
}

void IntMetaData::parsimUnpack(omnetpp::cCommBuffer *b)
{
    doParsimUnpacking(b,this->hopName);
    doParsimUnpacking(b,this->ts);
    doParsimUnpacking(b,this->qLen);
    doParsimUnpacking(b,this->txBytes);
    doParsimUnpacking(b,this->b);
    doParsimUnpacking(b,this->averageRtt);
    doParsimUnpacking(b,this->numOfFlows);
    doParsimUnpacking(b,this->rxQlen);
    doParsimUnpacking(b,this->numOfFlowsInInitialPhase);
}

const char * IntMetaData::getHopName() const
{
    return this->hopName.c_str();
}

void IntMetaData::setHopName(const char * hopName)
{
    this->hopName = hopName;
}

::omnetpp::simtime_t IntMetaData::getTs() const
{
    return this->ts;
}

void IntMetaData::setTs(::omnetpp::simtime_t ts)
{
    this->ts = ts;
}

long IntMetaData::getQLen() const
{
    return this->qLen;
}

void IntMetaData::setQLen(long qLen)
{
    this->qLen = qLen;
}

long IntMetaData::getTxBytes() const
{
    return this->txBytes;
}

void IntMetaData::setTxBytes(long txBytes)
{
    this->txBytes = txBytes;
}

long IntMetaData::getB() const
{
    return this->b;
}

void IntMetaData::setB(long b)
{
    this->b = b;
}

double IntMetaData::getAverageRtt() const
{
    return this->averageRtt;
}

void IntMetaData::setAverageRtt(double averageRtt)
{
    this->averageRtt = averageRtt;
}

int IntMetaData::getNumOfFlows() const
{
    return this->numOfFlows;
}

void IntMetaData::setNumOfFlows(int numOfFlows)
{
    this->numOfFlows = numOfFlows;
}

long IntMetaData::getRxQlen() const
{
    return this->rxQlen;
}

void IntMetaData::setRxQlen(long rxQlen)
{
    this->rxQlen = rxQlen;
}

int IntMetaData::getNumOfFlowsInInitialPhase() const
{
    return this->numOfFlowsInInitialPhase;
}

void IntMetaData::setNumOfFlowsInInitialPhase(int numOfFlowsInInitialPhase)
{
    this->numOfFlowsInInitialPhase = numOfFlowsInInitialPhase;
}

class IntMetaDataDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_hopName,
        FIELD_ts,
        FIELD_qLen,
        FIELD_txBytes,
        FIELD_b,
        FIELD_averageRtt,
        FIELD_numOfFlows,
        FIELD_rxQlen,
        FIELD_numOfFlowsInInitialPhase,
    };
  public:
    IntMetaDataDescriptor();
    virtual ~IntMetaDataDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(IntMetaDataDescriptor)

IntMetaDataDescriptor::IntMetaDataDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::IntMetaData)), "omnetpp::cObject")
{
    propertyNames = nullptr;
}

IntMetaDataDescriptor::~IntMetaDataDescriptor()
{
    delete[] propertyNames;
}

bool IntMetaDataDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<IntMetaData *>(obj)!=nullptr;
}

const char **IntMetaDataDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *IntMetaDataDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int IntMetaDataDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 9+base->getFieldCount() : 9;
}

unsigned int IntMetaDataDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_hopName
        FD_ISEDITABLE,    // FIELD_ts
        FD_ISEDITABLE,    // FIELD_qLen
        FD_ISEDITABLE,    // FIELD_txBytes
        FD_ISEDITABLE,    // FIELD_b
        FD_ISEDITABLE,    // FIELD_averageRtt
        FD_ISEDITABLE,    // FIELD_numOfFlows
        FD_ISEDITABLE,    // FIELD_rxQlen
        FD_ISEDITABLE,    // FIELD_numOfFlowsInInitialPhase
    };
    return (field >= 0 && field < 9) ? fieldTypeFlags[field] : 0;
}

const char *IntMetaDataDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "hopName",
        "ts",
        "qLen",
        "txBytes",
        "b",
        "averageRtt",
        "numOfFlows",
        "rxQlen",
        "numOfFlowsInInitialPhase",
    };
    return (field >= 0 && field < 9) ? fieldNames[field] : nullptr;
}

int IntMetaDataDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "hopName") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "ts") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "qLen") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "txBytes") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "b") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "averageRtt") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "numOfFlows") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "rxQlen") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "numOfFlowsInInitialPhase") == 0) return baseIndex + 8;
    return base ? base->findField(fieldName) : -1;
}

const char *IntMetaDataDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_hopName
        "omnetpp::simtime_t",    // FIELD_ts
        "long",    // FIELD_qLen
        "long",    // FIELD_txBytes
        "long",    // FIELD_b
        "double",    // FIELD_averageRtt
        "int",    // FIELD_numOfFlows
        "long",    // FIELD_rxQlen
        "int",    // FIELD_numOfFlowsInInitialPhase
    };
    return (field >= 0 && field < 9) ? fieldTypeStrings[field] : nullptr;
}

const char **IntMetaDataDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *IntMetaDataDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int IntMetaDataDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void IntMetaDataDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'IntMetaData'", field);
    }
}

const char *IntMetaDataDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string IntMetaDataDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        case FIELD_hopName: return oppstring2string(pp->getHopName());
        case FIELD_ts: return simtime2string(pp->getTs());
        case FIELD_qLen: return long2string(pp->getQLen());
        case FIELD_txBytes: return long2string(pp->getTxBytes());
        case FIELD_b: return long2string(pp->getB());
        case FIELD_averageRtt: return double2string(pp->getAverageRtt());
        case FIELD_numOfFlows: return long2string(pp->getNumOfFlows());
        case FIELD_rxQlen: return long2string(pp->getRxQlen());
        case FIELD_numOfFlowsInInitialPhase: return long2string(pp->getNumOfFlowsInInitialPhase());
        default: return "";
    }
}

void IntMetaDataDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        case FIELD_hopName: pp->setHopName((value)); break;
        case FIELD_ts: pp->setTs(string2simtime(value)); break;
        case FIELD_qLen: pp->setQLen(string2long(value)); break;
        case FIELD_txBytes: pp->setTxBytes(string2long(value)); break;
        case FIELD_b: pp->setB(string2long(value)); break;
        case FIELD_averageRtt: pp->setAverageRtt(string2double(value)); break;
        case FIELD_numOfFlows: pp->setNumOfFlows(string2long(value)); break;
        case FIELD_rxQlen: pp->setRxQlen(string2long(value)); break;
        case FIELD_numOfFlowsInInitialPhase: pp->setNumOfFlowsInInitialPhase(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntMetaData'", field);
    }
}

omnetpp::cValue IntMetaDataDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        case FIELD_hopName: return pp->getHopName();
        case FIELD_ts: return pp->getTs().dbl();
        case FIELD_qLen: return (omnetpp::intval_t)(pp->getQLen());
        case FIELD_txBytes: return (omnetpp::intval_t)(pp->getTxBytes());
        case FIELD_b: return (omnetpp::intval_t)(pp->getB());
        case FIELD_averageRtt: return pp->getAverageRtt();
        case FIELD_numOfFlows: return pp->getNumOfFlows();
        case FIELD_rxQlen: return (omnetpp::intval_t)(pp->getRxQlen());
        case FIELD_numOfFlowsInInitialPhase: return pp->getNumOfFlowsInInitialPhase();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'IntMetaData' as cValue -- field index out of range?", field);
    }
}

void IntMetaDataDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        case FIELD_hopName: pp->setHopName(value.stringValue()); break;
        case FIELD_ts: pp->setTs(value.doubleValue()); break;
        case FIELD_qLen: pp->setQLen(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_txBytes: pp->setTxBytes(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_b: pp->setB(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_averageRtt: pp->setAverageRtt(value.doubleValue()); break;
        case FIELD_numOfFlows: pp->setNumOfFlows(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_rxQlen: pp->setRxQlen(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_numOfFlowsInInitialPhase: pp->setNumOfFlowsInInitialPhase(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntMetaData'", field);
    }
}

const char *IntMetaDataDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr IntMetaDataDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void IntMetaDataDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    IntMetaData *pp = omnetpp::fromAnyPtr<IntMetaData>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntMetaData'", field);
    }
}

Register_Class(IntTag)

IntTag::IntTag() : ::inet::TagBase()
{
}

IntTag::IntTag(const IntTag& other) : ::inet::TagBase(other)
{
    copy(other);
}

IntTag::~IntTag()
{
}

IntTag& IntTag::operator=(const IntTag& other)
{
    if (this == &other) return *this;
    ::inet::TagBase::operator=(other);
    copy(other);
    return *this;
}

void IntTag::copy(const IntTag& other)
{
    this->connId = other.connId;
    this->intData = other.intData;
    this->rtt = other.rtt;
    this->cwnd = other.cwnd;
    this->initialPhase = other.initialPhase;
}

void IntTag::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::TagBase::parsimPack(b);
    doParsimPacking(b,this->connId);
    doParsimPacking(b,this->intData);
    doParsimPacking(b,this->rtt);
    doParsimPacking(b,this->cwnd);
    doParsimPacking(b,this->initialPhase);
}

void IntTag::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::TagBase::parsimUnpack(b);
    doParsimUnpacking(b,this->connId);
    doParsimUnpacking(b,this->intData);
    doParsimUnpacking(b,this->rtt);
    doParsimUnpacking(b,this->cwnd);
    doParsimUnpacking(b,this->initialPhase);
}

long IntTag::getConnId() const
{
    return this->connId;
}

void IntTag::setConnId(long connId)
{
    this->connId = connId;
}

const IntDataVec& IntTag::getIntData() const
{
    return this->intData;
}

void IntTag::setIntData(const IntDataVec& intData)
{
    this->intData = intData;
}

::omnetpp::simtime_t IntTag::getRtt() const
{
    return this->rtt;
}

void IntTag::setRtt(::omnetpp::simtime_t rtt)
{
    this->rtt = rtt;
}

unsigned int IntTag::getCwnd() const
{
    return this->cwnd;
}

void IntTag::setCwnd(unsigned int cwnd)
{
    this->cwnd = cwnd;
}

bool IntTag::getInitialPhase() const
{
    return this->initialPhase;
}

void IntTag::setInitialPhase(bool initialPhase)
{
    this->initialPhase = initialPhase;
}

class IntTagDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_connId,
        FIELD_intData,
        FIELD_rtt,
        FIELD_cwnd,
        FIELD_initialPhase,
    };
  public:
    IntTagDescriptor();
    virtual ~IntTagDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(IntTagDescriptor)

IntTagDescriptor::IntTagDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(inet::IntTag)), "inet::TagBase")
{
    propertyNames = nullptr;
}

IntTagDescriptor::~IntTagDescriptor()
{
    delete[] propertyNames;
}

bool IntTagDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<IntTag *>(obj)!=nullptr;
}

const char **IntTagDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *IntTagDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int IntTagDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int IntTagDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_connId
        FD_ISCOMPOUND,    // FIELD_intData
        FD_ISEDITABLE,    // FIELD_rtt
        FD_ISEDITABLE,    // FIELD_cwnd
        FD_ISEDITABLE,    // FIELD_initialPhase
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *IntTagDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "connId",
        "intData",
        "rtt",
        "cwnd",
        "initialPhase",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int IntTagDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "connId") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "intData") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "rtt") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "cwnd") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "initialPhase") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *IntTagDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "long",    // FIELD_connId
        "inet::IntDataVec",    // FIELD_intData
        "omnetpp::simtime_t",    // FIELD_rtt
        "unsigned int",    // FIELD_cwnd
        "bool",    // FIELD_initialPhase
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **IntTagDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *IntTagDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int IntTagDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void IntTagDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'IntTag'", field);
    }
}

const char *IntTagDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string IntTagDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        case FIELD_connId: return long2string(pp->getConnId());
        case FIELD_intData: return "";
        case FIELD_rtt: return simtime2string(pp->getRtt());
        case FIELD_cwnd: return ulong2string(pp->getCwnd());
        case FIELD_initialPhase: return bool2string(pp->getInitialPhase());
        default: return "";
    }
}

void IntTagDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        case FIELD_connId: pp->setConnId(string2long(value)); break;
        case FIELD_rtt: pp->setRtt(string2simtime(value)); break;
        case FIELD_cwnd: pp->setCwnd(string2ulong(value)); break;
        case FIELD_initialPhase: pp->setInitialPhase(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntTag'", field);
    }
}

omnetpp::cValue IntTagDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        case FIELD_connId: return (omnetpp::intval_t)(pp->getConnId());
        case FIELD_intData: return omnetpp::toAnyPtr(&pp->getIntData()); break;
        case FIELD_rtt: return pp->getRtt().dbl();
        case FIELD_cwnd: return (omnetpp::intval_t)(pp->getCwnd());
        case FIELD_initialPhase: return pp->getInitialPhase();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'IntTag' as cValue -- field index out of range?", field);
    }
}

void IntTagDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        case FIELD_connId: pp->setConnId(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_rtt: pp->setRtt(value.doubleValue()); break;
        case FIELD_cwnd: pp->setCwnd(omnetpp::checked_int_cast<unsigned int>(value.intValue())); break;
        case FIELD_initialPhase: pp->setInitialPhase(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntTag'", field);
    }
}

const char *IntTagDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_intData: return omnetpp::opp_typename(typeid(IntDataVec));
        default: return nullptr;
    };
}

omnetpp::any_ptr IntTagDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        case FIELD_intData: return omnetpp::toAnyPtr(&pp->getIntData()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void IntTagDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    IntTag *pp = omnetpp::fromAnyPtr<IntTag>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'IntTag'", field);
    }
}

}  // namespace inet

namespace omnetpp {

}  // namespace omnetpp

