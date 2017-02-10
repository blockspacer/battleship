#include <gtest/gtest.h>
#include "committer.hpp"
#include "dummy_connection.hpp"
#include "hash_committer.hpp"
#include "util.hpp"

template <typename T>
class HashCommitterTest : public ::testing::Test
{
protected:
    HashCommitterTest()
    {
        auto conn_pair{DummyConnection::make_dummies()};
        comm_sender = std::make_unique<T>(conn_pair.first);
        comm_receiver = std::make_unique<T>(conn_pair.second);
    }
    virtual ~HashCommitterTest() = default;

    std::unique_ptr<HashCommitter> comm_sender;
    std::unique_ptr<HashCommitter> comm_receiver;
};


TYPED_TEST_CASE_P(HashCommitterTest);


TYPED_TEST_P(HashCommitterTest, MakeVerify)
{
    bytes_t value(16);
    auto comm = this->comm_sender->make_commitment(value);
    ASSERT_EQ(value, comm->value);
    ASSERT_TRUE(this->comm_sender->verify_commitment(comm));

    value = bytes_t(16, 0x42);
    comm = this->comm_sender->make_commitment(value);
    ASSERT_EQ(value, comm->value);
    ASSERT_TRUE(this->comm_sender->verify_commitment(comm));
}


REGISTER_TYPED_TEST_CASE_P(
        HashCommitterTest,
        MakeVerify);


typedef ::testing::Types<SHA1Committer> HashCommitterImplementations;

INSTANTIATE_TYPED_TEST_CASE_P(HashCommitterTestInstance, HashCommitterTest, HashCommitterImplementations);