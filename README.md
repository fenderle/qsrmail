**This repository is no longer maintained and read-only.**

# QsrMail - a Qt based SMTP client library

### Introduction

QsrMail aimes to be a library which brings SMTP support to the Qt framework.
The library supports the following features:

- supports MIME messages with attachments
- supports QIODevice as attachment source
- provides Base64 and Quoted Printable attachment encoding
- provides Content-Type detection through QMimeDatabase
- supports TLS encryption
- supports SMTP authentication using CRAM-MD5, PLAIN and LOGIN mechs
- complete async design with a small memory footprint
- complete Doxygen documentation with integration into QtCreator
- comes with a demo application
- has been built and tested on Linux, OS-X and Windows (using Qt5.3 and 
  MSVC2013 from Visual Studio 2013 Express)

However the library requires Qt5 and does not compile with Qt4 due to some
missing features required. Qt4 support is not planned as of now.

### Getting started

To compile QsrMail you'll need Qt5 - nothing else. There is no other 
dependency.

If you compile QsrMail it will always compile a debug and a release build
and the libraries will be put into the lib directory in the project root.
Depending if you compile a static library version (using CONFIG+=static) 
or the standard dynamic library build the output files will end up in
lib/dynamic/... of lib/static/...:

    qsrmail/lib/dynamic:
      libqsrmaild.so                  <-- dll debug build
      libqsrmail.so                   <-- dll release build 
    qsrmail/lib/static:
      libqsrmaild.a                   <-- static debug build
      libqsrmail.a                    <-- static release build

To use the library in your project just include the pri file within your
.pro file:

    TestApp.pro:
    ...
    #CONFIG += qsrmail_static
    include(../qsrmail/qsrmail.pri)   <-- path to the qsrmail.pri file
    ...

If you uncomment the CONFIG+=qsrmail_static line the static library will
be linked instead of the dll version.

To compile the demo:

- open Qt Creator
- load the qsrmail/qsrmail.pro project
- add the qsrmail/examples/TestApp/TestApp.pro project
- since the TestApp project depends on qsrmail you'll have to build the
  library first

The demo application is a little application which lets you send mail to
through a SMTP server and is intented to be a simple playground, where
you can explore the API.

### The documentation

To build the documentation you need doxygen and graphviz. The build is 
straightforward:

    $ cd qsrmail/doxygen
    $ doxygen Doxyfile.public
    $ doxygen Doxyfile.internal

This will create two new directories *public* and *internal*. Each one
contains an *index.html* file. Just open that in your browser.

- *public* is the normal end user documentation while
- *internal* also documents the internal classes and provides diagrams
  for the state machines

You can also integrate the documentation into QtCreator. Therefore in
QtCreator go to the menu 'Tools -> Options', select 'Help' from the
lefthand list, change to the 'Documentation' tab and click 'Add...'.
Navigate to the file *qsrmail/doxygen/public/qsrmail.qch* and click
'Open'. Click 'Ok' to close the dialog and restart QtCreator. After that
the documentation should be available just as any other Qt documentation.
(Tested with QtCreator 3.2)

For your convenience the qch files are prebuilt and available in the
git repository.

## Licensing

QsrMail is licensed under LGPLv3.
