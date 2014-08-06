#ifndef TEST_H
#define TEST_H

#include <QSharedDataPointer>

class testData;

class test
{
public:
    test();
    test(const test &);
    test &operator=(const test &);
    ~test();

private:
    QSharedDataPointer<testData> data;
};

#endif // TEST_H
