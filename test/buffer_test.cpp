#include <string>

#include <eventpp/buffer.hpp>

#include "test.h"

using eventpp::Buffer;
using std::string;

TEST_CASE("testBufferAppendRead")
{
    Buffer buf;
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    const string str(200, 'x');
    buf.Append(str);
    REQUIRE(buf.length() == str.size());
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - str.size());
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    const string str2 = buf.NextString(50);
    REQUIRE(str2.size() == 50);
    REQUIRE(buf.length() == str.size() - str2.size());
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - str.size());
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + str2.size());
    REQUIRE(str2 == string(50, 'x'));

    buf.Append(str);
    REQUIRE(buf.length() == 2 * str.size() - str2.size());
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 2 * str.size());
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + str2.size());

    const string str3 = buf.NextAllString();
    REQUIRE(str3.size() == 350);
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
    REQUIRE(str3 == string(350, 'x'));
}

TEST_CASE("testBufferGrow1")
{
    Buffer buf;
    buf.Append(string(400, 'y'));
    REQUIRE(buf.length() == 400);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 400);

    buf.Retrieve(50);
    REQUIRE(buf.length() == 350);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 400);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 50);

    buf.Append(string(1000, 'z'));
    REQUIRE(buf.length() == 1350);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Reset();
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
    REQUIRE(buf.WritableBytes() >= Buffer::kInitialSize * 2);
}

TEST_CASE("testBufferGrow2")
{
    size_t prepend_size = 16;
    Buffer buf(Buffer::kInitialSize, prepend_size);
    buf.Append(string(400, 'y'));
    REQUIRE(buf.length() == 400);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 400);
    REQUIRE(buf.PrependableBytes() == prepend_size);

    buf.Retrieve(50);
    REQUIRE(buf.length() == 350);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 400);
    REQUIRE(buf.PrependableBytes() == prepend_size + 50);

    buf.Append(string(1000, 'z'));
    REQUIRE(buf.length() == 1350);
    REQUIRE(buf.PrependableBytes() == prepend_size);

    buf.Reset();
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.PrependableBytes() == prepend_size);
    REQUIRE(buf.WritableBytes() >= Buffer::kInitialSize * 2);
}

TEST_CASE("testBufferInsideGrow")
{
    Buffer buf;
    buf.Append(string(800, 'y'));
    REQUIRE(buf.length() == 800);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 800);

    buf.Retrieve(500);
    REQUIRE(buf.length() == 300);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 800);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 500);

    buf.Append(string(300, 'z'));
    REQUIRE(buf.length() == 600);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 600);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
}

TEST_CASE("testBufferShrink")
{
    Buffer buf;
    buf.Append(string(2000, 'y'));
    REQUIRE(buf.length() == 2000);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Retrieve(1500);
    REQUIRE(buf.length() == 500);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 1500);

    buf.Shrink(0);
    REQUIRE(buf.length() == 500);
    REQUIRE(buf.WritableBytes() == 0UL);
    REQUIRE(buf.NextAllString() == string(500, 'y'));
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
}

TEST_CASE("testBufferPrepend")
{
    Buffer buf;
    buf.Append(string(200, 'y'));
    REQUIRE(buf.length() == 200);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 200);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    int x = 0;
    buf.Prepend(&x, sizeof x);
    REQUIRE(buf.length() == 204);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 200);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize - 4);
}

TEST_CASE("testBufferReadInt")
{
    Buffer buf;
    buf.Append("HTTP");

    REQUIRE(buf.length() == 4);
    REQUIRE(buf.PeekInt8() == 'H');
    int top16 = buf.PeekInt16();
    REQUIRE(top16 == 'H' * 256 + 'T');
    REQUIRE(buf.PeekInt32() == top16 * 65536 + 'T' * 256 + 'P');

    REQUIRE(buf.ReadInt8() == 'H');
    REQUIRE(buf.ReadInt16() == 'T' * 256 + 'T');
    REQUIRE(buf.ReadInt8() == 'P');
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize);

    buf.AppendInt8(-1);
    buf.AppendInt16(-2);
    buf.AppendInt32(-3);
    REQUIRE(buf.length() == 7);
    REQUIRE(buf.ReadInt8() == -1);
    REQUIRE(buf.ReadInt16() == -2);
    REQUIRE(buf.ReadInt32() == -3);
    REQUIRE(buf.length() == 0);
}

TEST_CASE("testBufferFindEOL")
{
    Buffer buf;
    buf.Append(string(100000, 'x'));
    const char * null = nullptr;
    REQUIRE(buf.FindEOL() == null);
    REQUIRE(buf.FindEOL(buf.data() + 90000) == null);
}


TEST_CASE("testBufferTruncate")
{
    Buffer buf;
    buf.Append("HTTP");

    REQUIRE(buf.length() == 4);
    buf.Truncate(3);
    REQUIRE(buf.length() == 3);
    buf.Truncate(2);
    REQUIRE(buf.length() == 2);
    buf.Truncate(1);
    REQUIRE(buf.length() == 1);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 1);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
    buf.Truncate(0);
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Append("HTTP");
    buf.Reset();
    REQUIRE(buf.length() == 0);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Append("HTTP");
    buf.Truncate(Buffer::kInitialSize + 1000);
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 4);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Append("HTTPS");
    REQUIRE(buf.length() == 9);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 9);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Next(4);
    REQUIRE(buf.length() == 5);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 9);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 4);
    buf.Truncate(5);
    REQUIRE(buf.length() == 5);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 9);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 4);
    buf.Truncate(6);
    REQUIRE(buf.length() == 5);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 9);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 4);
    buf.Truncate(4);
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 8);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 4);
}


TEST_CASE("testBufferReserve")
{
    Buffer buf;
    buf.Append("HTTP");
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 4);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Reserve(100);
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 4);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);


    buf.Reserve(Buffer::kInitialSize);
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() == Buffer::kInitialSize - 4);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Reserve(2 * Buffer::kInitialSize);
    REQUIRE(buf.length() == 4);
    REQUIRE(buf.WritableBytes() >= 2 * Buffer::kInitialSize - 4);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Append("HTTPS");
    REQUIRE(buf.length() == 9);
    REQUIRE(buf.WritableBytes() >= 2 * Buffer::kInitialSize - 9);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);

    buf.Next(4);
    REQUIRE(buf.length() == 5);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize + 4);

    buf.Reserve(8 * Buffer::kInitialSize);
    REQUIRE(buf.length() == 5);
    REQUIRE(buf.WritableBytes() >= 8 * Buffer::kInitialSize - 5);
    REQUIRE(buf.PrependableBytes() == Buffer::kCheapPrependSize);
}
