W3Mfc 2.08 A collection of MFC classes to implement a Web Server, 
CWSocket v1.39 A set of C++ wrapper class for Sockets,
CThreadPoolServer v1.45 A C++ class framework for Thread pools,
SocServer v1.06 A simple demo server app for CWSocket &
SocClient v1.06 A simple demo client app for CWSocket
Welcome to W3MFC, a collection of MFC classes to implement a simple Web server.

The idea behind is W3MFC was originally to learn about the HTTP protocol and how it is implemented on Win32. It is not intended to be a replacement for IIS or Apache. Instead it is designed as a tool for learning or in cases where deployment of a solution on IIS would be considered overkill.

For detailed information about the Hyper Text Transfer Protocol you should read RFC 1945, You can download this from a number of sites by searching for RFC and 1945.

Another document that these classes refer to is RFC 2045 which defines MIME.

 

The classes which constitute W3Mfc are:

CW3MFCServerSettings: This class contains the settings which are used to configure the web server. Example member variables in this class include the virtual directories and the their default filenames. If you are developing a stand alone web server based on W3Mfc, you could for example store all the variables for the CW3MFCServerSettings instance in an ini file or in the registry.

 

CW3MFCServer: This is the actual class which implements the web server. It has a number of very simple functions to allow you to control the state of the web server such as Start and Stop. Internally a background thread is spun off to handle the client connections.

 

CWSocket: This is a simple C++/MFC encapsulation of an SDK socket. A class called CW3MFCSocket is derived for this in the W3MFC web server to provide custom HTTP parsing. The CWSocket class can be using in other projects to provide a simple encapsulation of a socket.

 

CW3MFCResponseHeader: This class helps in the handling of sending request headers when returning the HTTP responses to clients. It provides a number of methods to allow standard Http headers to be added to the client response.

 

CW3MFCMimeManager: This class is used when returning client responses. It is used to determine to mime type of a file given its extension type. This information is taken from the registry and is cached in this class. For example files of .htm or .html will normally be of mime type "text/html".

 

CW3MFCRequest: This class represents a request from a client. It contains information such as the URI of the request, the Http Verb used and the Http version used for the request. A member of this type is stored in the CW3MFCClient class.

 

CW3MFCClient: This class is used in the CW3MFCServer class to handle client connections. It handles the parsing of client requests and returning the appropriate response. A number of virtual functions are provided to allow end user customisation.

 

CW3MFCDirectory: This class is used by the CW3MFCClient class to allow per directory customisation of client connections. A number of virtual functions are provided to allow end user customisation.

 

CThreadPoolServer: This class provides the thread pool functionality. To use it in your code, just derive a class from CThreadPoolClient (as is done for the W3MFC web server) and implement your own specific code in CThreadPoolClient::Run. The CThreadPoolServer class can be used in other projects to provide a simple encapsulation of a thread pool.

 

The enclosed zip file contains the W3Mfc source code and a simple console based application which implements a simple web server.

 

Two additional downloads are provided as samples for the CWSocket class. socserver.zip implements a simple single threaded time (RFC 868) sockets server and socclient.zip implements a client for the same. Please note that if you want to compile these examples, then you should copy the SocMFC.cpp/h files from the main W3MFC zip file into the directory where you have downloaded the socclient and socserver samples.

 

Copyright
You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) when your product is released in binary form.
You are allowed to modify the source code in any way you want except you cannot modify the copyright details at the top of each module.
If you want to distribute source code with your application, then you are only allowed to distribute versions released by the author. This is to maintain a single distribution point for the source code.
 

Updates
v2.08 of W3MFC / v1.39 of CWSocket / v1.45 of CThreadPoolServer (10 December 2017)

Updated the code to compile cleanly when _ATL_NO_AUTOMATIC_NAMESPACE is defined.
Removed support for HTTP Keep-Alives. Removing this functionality means that the thread pool is W3MFC no longer becomes blocked waiting for subsequent requests. Thanks to Olaf Rabus for reporting this issue.
Replaced NULL throughout the codebase with nullptr.
The stack size of the listen thread can now be customized via CW3MFCSettings::m_nListenThreadStackSize. Thanks to Olaf Rabus for suggesting this update.
Removed NTLM support from W3MFC as it requires Keep-Alives.
v2.07 of W3MFC / V1.06 of SocClient and SocServer (13 August 2017)

Fixed an incorrect ASSERT in CW3MFCClient::ReturnErrorMessage
Fixed a compile error in CW3MFCClient::PostLog when CWSOCKET_MFC_EXTENSIONS is defined.
Replaced CString::operator LPC*STR() calls throughout the codebase with CString::GetString calls
Fixed up an issue in CreateFile call in CW3MFCDirectory::TransmitFile where the wrong file sharing flags were used.
v2.06 of W3MFC (30 June 2017)

Updated copyright details.
Updated the declaration of MFC synchronisation variables to work when ATL's atlsync.h is included in your project.
v2.05 of W3MFC / v1.38 of CWSocket / v1.43 of CThreadPoolServer (28 April 2017)

Updated the code to compile cleanly using /permissive-.
v2.04 of W3MFC / v1.42 of CThreadPoolServer (17 April 2017)

Updated copyright details.
Updated the sample app and W3MFC to support a queue size different to the thread pool size. This new value is available as CW3MFCSettings::m_nThreadPoolQueueSize
v2.03 of W3MFC / v1.41 of CThreadPoolServer (22 November 2016)

Removed unnecessary inclusion of atlcoll.h in stdafx.h from sample app.
Any errors which occur in the listener worker thread in CW3MFCServer::ListenSocketFunction are now correctly reported back to the main thread which starts it.
The last error value is now set correctly upon return in CIOCPThreadPoolQueue::Create.
The last error value is now set correctly upon return in CDirectedThreadPoolQueue::Create.
Fixed a bug where the waitable timer is not reset correctly in the CW3MFCSocket::ReadRequest method.
v2.02 of W3MFC (17 August 2016)

Minor update to remove some unreferenced header files includes as well as warnings if you do not have MFC included in your pre-compiled headers.
Reworked CW3MFCClient class to be inherited from SSLWrappers::CSocket. This allows for easier overloading of the various SSLWrappers class framework virtual functions.
v1.40 of CThreadPoolServer (11 August 2016)

Fixed two handle leaks in the CThreadPoolClient destructor. Thanks to Robin Hilliard for reporting this bug.
v2.01 of W3MFC (24 July 2016)

Reworked CW3MFCRequestParams class to be derived from CMapStringToString
Removed call to m_HeaderMap.InitHashTable in CW3MFCRequest constructor.
V1.05 of SocClient and v1.05 of SocServer (24 July 2016)

Updated copyright details
Updated code to compile cleanly in VC 2010 to VC 2015
v2.0 of W3MFC / v1.37 of CWSocket / v1.39 of CThreadPoolServer (17 July 2016)

Updated copyright details
Added SAL annotations to the CWSocketException and CWSocket classes.
Reimplemented CW3MFCClient::UTF8ToCString to use the MultiByteToWideChar Win32 API.
Fixed a typo in the definition of the preprocessor value which decides on MFC integration in the socket class. The preprocessor value has been changed from CWSOCKET_MFC_EXTENSTIONS to CWSOCKET_MFC_EXTENSIONS.
Added SAL annotations to the CDirectedThreadPoolQueue class.
Added SAL annotations to the CIOCPThreadPoolQueue class.
Added SAL annotations to the CThreadPoolRequest, CThreadPoolQueue, CThreadPoolClient & CThreadPoolServer classes.
Replaced CW3MFCClient::HexToInt implementation with calls to ATL::AtlHexValue.
Added a new CW3MFCRequestParams class which provides for parsing of m_sRawExtra into a key value collection. This new class is called CW3MFCRequestParams and is based on the ATL Server class "CHttpRequestParams" class.
CW3MFCDirectory::TransmitDirectory now encodes any filename as UTF8.
Updated CW3MFCDirectory::TransmitDirectory method to remove calls to VERIFY.
27 November 2015

Updated the download file to include missing VC2013 solution and project files. Thanks to Arsi Koutaniemi for reporting this issue
v1.99 of W3MFC / v1.36 of CWSocket (28 October 2015)

Updated the code to clean compile on VC 2015
Updated copyright details.
CWSocket::GetPeerName & CWSocket::GetSockName now have an extra default parameter called "nAddressToStringFlags" which is passed to the AddressToString method.
CWSocket::ReceiveFrom now has extra default parameter called "nAddressToStringFlags" which is passed to the AddressToString method.
v1.38 of CThreadPoolServer (4 January 2015)

Addition of CDirectedThreadPoolQueue::GetCriticalSection method.
Updated copyright details.
v1.98 of W3MFC / v1.35 of CWSocket (26 November 2014)

Removed CWSocket methods which were deprecated in previous versions of the code.
Updated the class to clean compile on VC 2013 Update 3 and higher .
Updated the code to use the Author's SSLWrappers classes (http://www.naughter.com/sslwrappers.html) to provide the SSL functionality for W3MFC instead of OpenSSL. Please note that the the W3MFC code when in SSL mode depends on the author's CryptoWrappers classes (http://www.naughter.com/cryptowrappers.html) also. You will need to download both of these libraries and copy their modules into the the W3MFC directory. Also note that the SSLWrappers and CryptoWrapper classes are only supported on VC 2013 or later and this means that the W3MFC SSL support is only supported when you compile with VC 2013 or later. The solution files included in the W3MFC download are now for VC 2013.
13 April 2014

The sample app is now linked against the latest OpenSSL v1.0.1g dlls. This version is the patched version of OpenSSL which does not suffer from the Heartbleed bug.
v1.97 of W3MFC / v1.37 of CThreadPoolServer (29 March 2014)

Reworked the CThreadPoolRequest, CThreadPoolQueue, CThreadPool, CIOCPThreadPoolQueue and CDirectedThreadPoolQueue classes to compile without MFC. To achieve this the internal dependencies on MFC have been removed resulting in some breaking changes. For example the CThreadPool class is now templatised on a TCLIENT and TQUEUE class instead of via CRuntimeClass values being passed as parameters for these to the CThreadPool::Start method. In addition the threads in the thread pool are creating using the CRT function _beginthreadex instead of via AfxBeginThread. Also all the dependencies on the MFC multithreading classes such as CCriticalSection and CEvent have been replaced by ATL or raw Win32 functionality. You can use the W3MFC sample web server source code to see the required client side changes to use the updated thread pool classes.
v1.96 of W3MFC / v1.34 of CWSocket / v1.36 of CThreadPoolServer (16 March 2014)

Updated copyright details
Updated the code to compile cleanly on VC 2013 & VC 20013.
The sample app is now linked against the latest OpenSSL v1.0.1f dlls.
Reviewed all ASSERT calls for correctness
Reviewed all AFXASSUME calls for correctness
Updated #endif comments throughout the code base
By default W3MFC now builds using the non MFC build configuration for CWSocket and CWSocketException
Removed defunct code in CW3MFCClient constructor for old versions of Windows
Removed support for TransmitPackets and TransmitFile APIs from W3MFC for simplicity reasons.
Reworked CW3MFCClient::TransmitFile method
Reworked CWSocket::Shutdown method to use standard SDK defines rather than enums in the class
Removed all the CWSocket proxy connection methods as they cannot be easily supported / tested by the author.
Reworked the CWSocket and CWSocketException classes to optionally compile without MFC. By default the classes now use STL classes and idioms but if you define CWSOCKET_MFC_EXTENSTIONS the classes will revert back to the MFC behaviour.
Reworked CWSocket::ReceiveFrom to use GetNameInfoW / getnameinfo.
Reworked CWSocket::AddressToString to use GetNameInfoW / getnameinfo.
CWSocket::AddressToString now also returns the port number
Provided an overloaded version of CWSocket::AddressToString which takes a SOCKADDR& parameter
CWSocket::AddressToString method now takes a nFlags parameter
The CWSocket::CreateAndConnect has been enhanced to include nFamily and nProtocolType parameters. This allows client code for example to explicitly connect to IPv4 or IPv6 addresses.
v1.95 of W3MFC / v1.33 of CWSocket / v1.35 of CThreadPoolServer (12 August 2012)

Updated copyright details
CSSL::Close now provides a bGracefully parameter
CSSLSocket::Close now provides a bGracefully parameter
Updated the code to compile cleanly on VC 2012
v1.32 of CWSocket / CSSLSocket (3 April 2011)

The state of whether a socket should be bound or not is now decided by a new m_sBindAddress member variable. This variable can be modified through new Get/SetBindAddress methods.
Updated W3MFC binary to now link to OpenSSL 1.0.0d.
v1.31 of CWSocket / CSSLSocket (8 February 2011)

The state of whether a socket should be bound or not is now decided by a new m_sBindAddress member variable. This variable can be modified through new Get/SetBindAddress methods.
Fixed a number of compile problems in VC 2005 related to ATL::CSocketAddr::GetAddrInfoList() return value.
Reinstated Sockv4, Socksv5 and HTTP proxy methods in SSL socket class
v1.94 of W3MFC / v1.30 of CWSocket / v1.34 of CThreadPoolServer / V1.04 of SocClient and SocServer (9 January 2011)

Updated copyright details
Updated code to compile with latest version of OpenSSL. The distributed binaries are now linked against the latest OpenSSL v1.0.0c dlls
Reworked the low level internals of the thread pool to implement message pumping in CDirectedThreadPoolQueue::GetRequest instead of CThreadPoolClient::Main. This avoids a thread deadlock problem in CThreadPoolClient::Main if you want to pump messages.
Fixed a runtime ASSERT issue when removing directed requests in CDirectedThreadPoolQueue::GetRequest.
Updated CWSocket::Create method which takes a BOOL to include another default parameter to indicate IPv6
Updated CWSocket::GetPeerName to operate for IPv6 as well as IPv4
All CWSocket::Connect methods now try to connect all addresses returned from address lookups
Addition of a CWSocket::CreateAndBind method which support IPv6 binding
CWSocket::ReceiveFrom(void* pBuf, int nBufLen, CString& sSocketAddress, UINT& nSocketPort, int nFlags) method has been updated to support IPv6.
CWSocket::SendTo(const void* pBuf, int nBufLen, UINT nHostPort, LPCTSTR pszHostAddress, int nFlags) method has been updated to support IPv6 as well as connecting to all addresses returned from address lookups.
Removed all _alloca calls
Addition of a number of CreateConnect methods which support IPv6
v1.93 of W3MFC

Fixed a debug mode ASSERT when calling TRACE in CW3MFCClient::PostLog
v1.92 of W3MFC / v1.29 of CWSocket (12 July 2009)

Code in CMyHttpClient::PostLog now correctly handles Daylight Savings when logging.
Copy constructors and assignment operators are now private to ensure that the CSSLContext, CSSL, & CSSLSocket classes are not copyable. The previous fix on 23-05-2009 was not sufficient as default compiler generated methods are created for these two C++ methods. Thanks to Dmitriy Maksimov for following up on this issue.
v1.91 of W3MFC / v1.28 of CWSocket/CSSLSocket / v1.33 of CThreadPoolServer / V1.03 of SocClient and SocServer (24 May 2009)

Updated copyright details
Updated the sample app's project settings to more modern default values.
Removed some outstanding VC 6 style code from the codebase
Removed the operator= and copy constructor support from the CSSLContext and CSSL classes. This avoids de-allocation problems in the respective destructor classes when one of these contexts is shared between two C++ class instances. Thanks to Dmitriy Maksimov for reporting this bug.
Removed use of CT2A throughout the code.
Reworked all token parsing code to use CString::Tokenize.
Updated W3MFC binary to now link to OpenSSL 0.9.8k.
v1.90 of W3MFC / V1.02 of SocClient and SocServer (8 June 2008)

Updated code to compile correctly using _ATL_CSTRING_EXPLICIT_CONSTRUCTORS define
Updated SocClient & SocServer examples to compile cleanly using using Code Analysis (/analyze)
v1.89 of W3MFC / v1.27 of CWSocket / v1.32 of CThreadPoolServer (31 May 2008)

Code now compiles cleanly using Code Analysis (/analyze)
Tidied up the CWSocket::ReadHTTPProxyResponse implementation
Replaced use of CW32Handle class with ATL::CHandle
v1.31 of CThreadPoolServer

Addition of a new CDirectedThreadPoolQueue::GetCurrentQueueSize method.
v1.88 of W3MFC / v1.26 of CWSocket (1 March 2008)

Since the code is now for VC 2005 or later only, the code now uses the Base64 encoding support from the ATL atlenc.h header file. Thanks to Mat Berchtold for reporting this optimization. This means that client projects no longer need to include Base64.cpp/h in their projects.
v1.87 of W3MFC / V1.30 of CThreadPoolServer / v1.25 of CWSocket (19 February 2008)

Removed all legacy VC 6 code from classes. All the modules are now for VC 2005 or later only.
Fixed a memory leak in CW3MFCClient::_TransmitFile
All methods of CBase64 class have been made static.
Fixed a 64 bit bug in CW3MFCDirectory::TransmitDirectory related to the 64 bit size returned from CFileFind::GetLength.
v1.86 of W3MFC / v1.24 of CWSocket (2 February 2008)

Updated copyright details.
Fixed potential heap memory leaks in CWSocket::ReadHTTPProxyResponse.Thanks to Michal Urbanczyk for reporting this bug.
Fixed a memory leak in CWSocket::ConnectViaSocks5
Restructured CWSocket::ReadSocks5ConnectReply to avoid the need to allocate heap memory
v1.85 of W3MFC / v1.23 of CWSocket (31 December 2007)

Minor coding updates to CWSocketException::GetErrorMessage
Updates sample apps to clean compile on VC 2008
v1.22 of CWSocket (27 December 2007)

CWSocketException::GetErrorMessage now uses Checked::tcsncpy_s similiar to the built in MFC exception classes
v1.84 of W3MFC / v1.21 of CWSocket (26 December 2007)

CWSocketException::GetErrorMessage now uses the FORMAT_MESSAGE_IGNORE_INSERTS flag. For more information please see Raymond Chen's blog at http://blogs.msdn.com/oldnewthing/archive/2007/11/28/6564257.aspx. Thanks to Alexey Kuznetsov for reporting this issue.
All username and password temp strings in CWSocket are now securely destroyed using SecureZeroMemory. This version of the code and onwards will be supported only on VC 2005 or later.
V1.83 of W3MFC / V1.29 of CThreadPoolServer / V1.02 of CW32Handle / v1.20 of CWSocket (20 November 2007)

THREADPOOL_SHUTDOWN_REQUEST and THREADPOOL_USER_DEFINED_REQUEST constants have now been made enums of the CThreadPoolRequest class.
Updated W3MFC binary to now link to OpenSSL 0.9.8g.
CHttpServer is no longer derived from CObject as it was not really required.
Updated CIOCPThreadPoolQueue class to clean compile on x64.
Updated copyright details.
Optimized CSSLContext constructor.
Optimized CSSL constructor.
Optimized CSSLSocket constructor.
CHttpResponseHeader class has been renamed CW3MFCResponseHeader.
CHttpMimeManager class has been renamed CW3MFCMimeManager.
CRegistryHttpMimeManager class has been renamed CW3MFCRegistryMimeManager.
CIniHttpMimeManager class has been renamed CW3MFCIniMimeManager.
CHttpISAPI class has been renamed to CW3MFCISAPI.
CHttpISAPIExtension class has been renamed to CW3MFCISAPIExtension.
CHttpDirectory class has been renamed to CW3MFCDirectory.
CHttpClient class has been renamed to CW3MFCClient.
CHttpCGI class has been renamed to CW3MFCCGI.
Project files which are now included in the download are for VC 2005 instead of VC 6.
Please note that some of the module names for various W3MFC classes have been renamed to follow the new naming convention. You will need to update your client projects to remove the old module names and include the new ones.
V1.82 of W3MFC (9 May 2007)

Updated copyright details.
Fixed a bug in the two CHttpSocket::ReadResponse methods where the code would forget to keep the received buffer null terminated if the buffers have to be grown while a HTTP request is being parsed. Also these methods have been renamed to ReadRequest. Thanks to Alberto Massari for reporting this particularly nasty bug.
V1.01 of SocClient and SocServer (27 January 2007)

Updated copyright details.
Updated the code to clean compile on VC 2005
V1.81 of W3MFC (12 January 2007)

Updated copyright details.
Optimized CHttpRequest constructor code.
Optimized CHttpServerSettings constructor code.
Fixed a simple copy and paste issue in the IDH_302 built in html resource. Thanks to G. Wilmes for reporting this issue.
Fixed a bug in CHttpResponseHeader::GetData to do with the calculation of the ASCII header size when the code is compiled for Unicode. Thanks to Alexey V. Leonov for reporting this bug.
Fixed a bug in CRegistryHttpMimeManager::GetMimeType where it did not correctly handle an empty file extension value. Thanks to Alexey V. Leonov for reporting this bug.
V1.80 of W3MFC (21 September 2006)

Code now securely disposes of CHttpServer::m_sUsername, CHttpServer::m_sPassword, CHttpRequest::m_sUserName, CHttpRequest::m_sPassword using SecureZeroMemory. Note that the CHttpDirectory::m_sUsername and CHttpDirectory::m_sPassword values are also securely cleaned up but these are long lived values and as such if a crash occurs, these values will most likely appear in any memory dumps.
The W3MFC binary which is include in the download is now built with VC 2005 and dynamically links to the latest version of OpenSSL namely 0.9.8c, which you will need to build yourself, if you want to use the W3MFC exe as is. Also bear in mind that the new exe requires the VC 2005 MFC support dlls to be in place for it to run on client machines.
V1.28 of CThreadPoolServer (8 August 2006)

Provision of a CThreadPoolClient::m_bPumpMessageQueue boolean. This allows you to specify that internally the thread pool client will use MsgWaitForMultipleObjects to wait for a request to appear on the thread pool queue or a windows message to appear on the thread's system queue. If a windows message is detected, then the thread pool will internally pump the message queue until no more messages are present. If CThreadPoolClient::m_bPumpMessageQueue is left at the default of FALSE, then you will get the old thread pool behaviour which is to directly call into CThreadPoolQueue::GetRequest which will block until a request is available. This new addition prevents problems if your thread pool initializes and uses COM as an STA. In this case the thread is required to pump messages since COM uses the message queue for synchronisation when it is initialized as an STA. If you do not pump the message queue when you are using COM as an STA, you will notice unusual behaviour such as the Windows shell being unresponsive when ShellExecute calls are running. This is because internally ShellExecute uses DDE which itself broadcasts window messages to all top level windows. When you call CoInitialize to use an STA, internally a top level window is created, but because there is no code servicing this windows message queue, the shell's ShellExecute calls will seem to hang for 30 seconds. Boy did it take me some work to find the root of this problem. You have been warned!!!
Addition of a CDirectedThreadPoolQueue::m_evtDataAvailable member variable which is signalled when an item is available on the queue
V1.27 of CThreadPoolServer (6 August 2006)

Minor update to fix a compile problem when compiled for x64
V1.79 of W3MFC / v1.19 of CWSocket / V1.26 of CThreadPoolServer (27 June 2006)

Code now uses new C++ style casts rather than old style C casts where necessary.
Updated copyright details.
Made AfxThrowWSocketException part of CWSocket class and renamed to ThrowWSocketException.
Optimized CWSocketException constructor code.
Removed unnecessary CWSocketException destructor.
Optimized CIOCPThreadPoolQueue constructor code.
Optimized CHttpISAPIExtension constructor code.
Optimized CHttpISAPI constructor code.
Optimized CHttpDirectory constructor code.
Removed unused CHttpDirectory destructor.
Optimized CHttpResponseHeader constructor code
The class framework now requires the Platform SDK if compiled using VC 6.
Optimized CThreadPoolClient constructor code.
Reworked thread recycling logic in CThreadPoolServer::Monitor to use a much simpler WaitForSingleObject call.
Optimized CThreadPoolServer constructor code.
Optimized CHttpClient constructor code.
Updated the code to clean compile on VC 2005.
V1.78 of W3MFC / V1.25 of CThreadPoolServer (31 May 2006)

Fixed a memory leak in CHttpServer::ListenSocketFunction which occurs when an exception is thrown because the client socket connection is not accepted or when the request fails to be sent down to the thread pool. Thanks to Daniel Berman for reporting this bug.
Added detailed comments in ThrdPool.h on the proper use of CThreadPoolRequest member variables. This information also includes detailed info on how to design your thread pool requests such that you avoid any potential memory leaps. Thanks to Daniel Berman for reporting this issue.
Removed the virtual destructor for CThreadPoolRequest as the class is not meant to be derived from. Also added comments to the same effect in ThrdPool.h.
V1.24 of CThreadPoolServer (13 April 2006)

Updated copyright details.
Provision of Lock and Unlock methods along the lines of the author's CSyncCollection class.
CDirectedThreadPoolQueue::PostRequest, PostRequestWithoutLimitCheck, Close and GetRequest now includes a parameter which decides if internal locking should be done. Again this allows more fine tuned external synchronisation of the class.
Made timeout value in PostRequest default to "INFINITE".
Made GetDirectedRequestIndexToRemove method virtual.
Optimized CDirectedThreadPoolQueue constructor code.
Renamed CThreadPoolClient::m_RequestToStop to m_evtRequestToStop.
V1.77 of W3MFC / v1.18 of CWSocket (19 February 2006)

Replaced all calls to ZeroMemory and CopyMemory with memset and memcpy.
V1.76 of W3MFC (8 February 2006)

Minor update to ensure that all code compiles cleanly when using /Wp64 setting. Thanks to Alexey Kuznetsov for prompting this update.
Updated the documentation to use the same style as the web site.
V1.75 of W3MFC (3 January 2006)

Updated the copyrights in all the W3MFC modules.
Fixed an issue in the CGI implementation so that HTTP headers such as User-Agent get mapped to HTTP_USER_AGENT environment variables. Basically all HTTP_... values should have any hyphens in their names replaced by underscores.
Now includes support for the DOCUMENT_ROOT CGI environment variable.
Now returns a HTTP status code of 302 if a "Content:" header is found but no status code is present. Thanks to Hector Santos for reporting this issue.
Now CGI implementation supports Complete (Non-Parsed) CGI scripts. If you prefix your CGI script with "NPR-" then the data as received from the script will be sent to the client unmodified. Thanks to Hector Santos for suggesting this nice addition.
Parsing of the HTTP status code line e.g. "HTTP/1.0 200 Ok" is now done using the CGI "Status:" header rather than looking for a "HTTP/.." line. Again thanks to Hector Santos for reporting this.
v1.17 of CWSocket (27 November 2005)

Updated comments in the header file to accompany inclusion of the OpenSSL header files.
v1.16 of CWSocket (16 November 2005)

CSSLSocket::Send now uses a const void* parameter.
V1.74 of W3MFC / v1.15 of CWSocket (4 November 2005)

Fixed a further bug in the CGI implementation where subsequent HTTP chunks which are transmitted to clients include an additional HTTP Status line. Thanks to Gregoire Gentil for reporting this persistant bug.
Fixed an issue where the returned HTTP status line by W3MFC did not have the correct EOL terminator.
CWSocket::Send method now returns the number of bytes written. Thanks to Owen O'Flaherty for pointing out this omission.
V1.73 of W3MFC (30 October 2005)

Fixed a bug in CHttpCGI::ReadFromClientStdout when parsing CGI responses which had embedded HTTP headers in them. Thanks to Gregoire Gentil for reporting this bug.
Shipped binary for W3MFC is now linked with the OpenSSL 0.9.7i which is the latest version currently.
V1.23 of CThreadPoolServer (11 September 2005)

Reworked the CThreadPoolClient::_Run method so that it can now be customized at runtime. There is now a non static version which is also called _Run and there is also a virtual Main function which implements the main loop of the thread pool.
A CEvent namely "m_RequestToStop" is now also set when the thread pool is being stopped.
V1.14 of CWSocket / CSSLSocket (21 June 2005)

Provision of connect methods which allows a timeout to be specified. Please note that if you use a host name in these calls as opposed to an IP address, the DNS lookup is still done using the OS supplied timeout. Only the actual connection to the server is implemented using a timeout after the DNS lookup is done (if it is necessary).
V1.13 of CWSocket / CSSLSocket (1 May 2005)

Send method now uses a const void* parameter.
V1.72 of W3MFC / v1.11 of CWSocket (19 February 2005)

Fixed a bug when directory lists were displayed by W3MFC. It incorrectly always dropped the last entry from the listing. Thanks to Pablo Romero for reporting this bug.
Provided a CSSLSocket::IsReadible(DWORD dwTimeout) which now correctly uses the SSL_pending function. This avoids problems with the read ahead buffer used by OpenSSL and the way W3MFC uses to detect if the socket is readible. Also reworked the CHttpSocket::ReadResponse function to use this SSL_pending function. Because of these changes, the SSL code path in W3MFC now does not use the event based mechanism for reading requests which the non SSL code path uses. Thanks to Leandro Gustavo Biss Becker for reporting this issue.
V1.11 of CWSocket / CSSLSocket (31 January 2005)

Fixed a bug in CWSocket::Receive where it throws an error when a graceful disconnect occurs. Now the code only throws an exception if the return value from recv is SOCKET_ERROR.
V1.10 of CWSocket / CSSLSocket (29 December 2004)

Almost all of the following updates were to match the functionality provided by the MFC CAsyncSocket class but without the overhead of hidden windows and its async behaviour.
Now automatically links to Winsock via #pragma comment
Addition of a GetPeerName method.
Replaced all calls to ZeroMemory to memset.
Addtion of a GetSockName method.
Addition of a SetSockOpt method.
Addition of a Flags parameter to Receive method.
Addition of a IOCtl method.
Optimized the code in Listen.
Addition of a ReceiveFrom method.
Addition of a ShutDown method.
Optimized the code in Close.
Remove of pszLocalBoundAddress parameter from Connect methods to make it consistent with CAsyncSocket.
Addition of a Flags parameter to Send method.
Optimized code in CWSocket destructor.
Addition of an overloaded Create method which allows all of the socket parameters to be set.
Use of _tcslen has been minimized when NULL string parameters can be passed to various CWSocket methods.
Change of various parameter names to be consistent with names as used in CAsyncSocket.
V1.22 of CThreadPoolServer (4 December 2004)

Updated the macro which detects if arrays use INT_PTR for index rather than int.
Also removed some ASSERTS which were overly restrictive in light of the queue now being externally modifiable via CThreadPoolServer::GetQueue.
V1.21 of CThreadPoolServer / V1.71 of W3MFC / V1.09 of CWSocket / V1.01 of CW32Handle (11 November 2004)

CWSocket: Updated to compile cleanly when Detect 64 bit issues and Force conformance in for loop options are enabled in Visual Studio .Net.
Updated all classes to define the respective preprocessor define to allow inclusion in an extension dll by default.
CW3MFC: Added a m_sRawExtra variable to CHttpRequest. This value is now passed to CGI and ISAPI instead of m_sExtra. This allows apps to correctly parse form parameters. Thanks to Jan Bares for reporting this problem.
CThreadPoolSerever: Provided a GetRequestArray() method in CDirectedThreadPoolQueue which allows access to the internal array used to hold the thread pool requests. Can prove handy to have access to this in certain circumstances.
CThreadPoolServer: Updated to compile cleanly when Detect 64 bit issues and Force conformance in for loop options are enabled in Visual Studio .Net
V1.70 of W3MFC (15 October 2004)

Removed an unnecessary ASSERT from CHttpClient::MapURLToLocalFilename. Thanks to Jan Bares for reporting this problem.
Changed the behaviour of the code in CHttpClient::MapURLToLocalFilename as follows:

Before Change

Requesting the URL "/name" would return the file "name" in the root directory of the web server even if a "/name" virtual directory existed. If "name" did not exist in the root directory then a 404 error would be returned

After Change

Requesting the URL "/name" will return the virtual directory "/name" if such a virtual directory exists. If not then the file "name" in the root directory will be returned.

Thanks to Jan Bares for pointing out this behaviour which is inconsistent with other Web Server implementations.
URLs which include "@" now are parsed correctly. Previously the code was parsing the URI expecting it to contain username and password in the URL itself. Instead when the URI arrives at the server it is not in the URI itself but is represented in the HTTP request headers. Thanks to Jan Bares for pointing out this problem.
Passthrough authentication can now be disabled via a new CHttpServerSettings::m_bPerformPassthroughAuthentication setting. This is useful where you want to setup per directory protection using a username / password pair but you do not want to use these credentials in an attempt to impersonate that user by calling the SDK function "LogonUser". Again thanks to Jan Bares for this suggestion.
6 September 2004

Minor update to the documentation.
V1.69 of W3MFC (26 August 2004)

The binaries included in the download now link against OpenSSL 0.9.7d.
Per thread cleanup is now done for OpenSSL. Again thanks to Leandro Gustavo Biss Becker for reporting this problem.
OpenSSL is now configured by the W3MFC sample app to operate in a thread safe manner. Again thanks to Leandro Gustavo Biss Becker for reporting this problem.
Application initialization of OpenSSL has been taken out of W3MFC and is now the responsibility of your application. See the sample W3MFC application code in main.cpp for an example on how to correctly initialize and terminate OpenSSL.
CSSL::Close() now calls SSL_shutdown to ensure SSL connections are properly terminated. Thanks to Leandro Gustavo Biss Becker for reporting this problem.
W3MFC now correctly handles UTF8 encoded URL requests. Thanks to Huang Wei for reporting this problem and providing the fix.
V1.0 of SocClient and SocServer (20 August 2004)

New samples which show very simple usage of the CWSocket wrapper class.
V1.68 of W3MFC (30 March 2004)

Tidied up the interaction of the various classes by removing all friend classes
V1.67 of W3MFC (22 February 2004)

Fixed a memory leak in CHttpISAPI::CachedLoad. Thanks to "mori" for reporting and suggesting the fix for this bug.
Updated the copyright messages of all the modules.
V1.66 of W3MFC / V1.20 of CThreadPoolServer / V1.08 of CWSocket (13 January 2004)

Made the m_bRequestToStop member variable "volatile" as it can be modified from multiple threads while possible been read in a loop in another thread. Thanks to Dan Baker for reporting this issue.
Used newer form of #pragma pack to avoid problems with non standard packing sizes.
V1.65 of W3MFC

URL Encoded spaces as "+" are now correctly handled. Thanks to Dan Baker for reporting this issue.
CHttpRequest now includes a m_sRawURL member variable which provides access to the raw URL before it is URL decoded. Thanks to Dan Baker for suggesting this addition.
Shipped binary for W3MFC is now linked with the OpenSSL 0.9.6l which is the latest version currently.
V1.64 of W3MFC / V1.07 of CWSocket (3 November 2003)

Simplified the code in CHttpSocket::ReadResponse. Thanks to Clarke Brunt for reporting this issue.
Simplified the code in CWSocket::ReadHTTPProxyResponse. Thanks to Clarke Brunt for reporting this issue.
V1.06 of CWSocket / CSSLSocket (5 October 2003)

Provided a new class called CSSLSocket which provides a socket wrapper class over SSL (via OpenSSL). In combination with the CWSocket class, you can now do all that CWSocket does such as Socks 4 and 5 and HTTP Proxy connections but implement the transmit / receive over a secure SSL connection. See the projects SSLClient and SSLServer in the workspace for a very simple SSL client and server pair.
V1.05 of CWSocket (26 September 2003)

Now supports connection via HTTP proxies which support the CONNECT verb.
V1.04 of CWSocket (21 September 2003)

Now supports UDP sockets.
Now supports UDP relaying via Socks 5.
V1.63 of W3MFC (12 September 2003)

Removed double definition of SCRIPT_NAME from CGI environment. Thanks to Dave Horner for reporting this.
SCRIPT_NAME CGI environment variable now uses forward slashes rather than back slashes as directory separators. Thanks to Dave Horner for reporting this problem.
Added a "REQUEST_URI" CGI environment variable. Thanks to Dave Horner for reporting this.
CGI implementation now returns a HTTP return code line if one if the CGI script does not provide one. Again thanks to Dave Horner for reporting this issue.
CGI implementation now returns immediately from CHttpCGI::ReadFromClientStdout from a socket error is detected. Again thanks to Dave Horner for reporting this issue.
"PATH_TRANSLATED" CGI and ISAPI environment variable is now reported as an absolute path. Again thanks to Dave Horner for reporting this issue.
"SCRIPT_NAME" CGI and ISAPI environment variable now includes an initial "/". Again thanks to Dave Horner for reporting this issue.
CGI now uses pClient->m_Request.m_dwRawEntitySize variable when determining when W3MFC needs to write to the CGI child process.
ISAPI implementation now sends just the HTTP body to client ISAPI dlls instead of the whole HTTP request. Thanks to Michael St. Laurent for reporting this problem.
6 April 2003

Updated CGI Sample app to return content-type in the HTTP header, no other changes made. Thanks to "mlcarey59" for reporting this
V1.03 of CWSocket (3 March 2003)

Addition of a number of preprocessor defines, namely W3MFC_EXT_CLASS, THRDPOOL_EXT_CLASS and SOCKMFC_EXT_CLASS. This allows the classes to easily be added and exported from a MFC extension dll.
Now implements support for connecting via Socks 4 and Socks 5 proxies
V1.19 of CThreadPoolServer (3 March 2003)

Addition of a number of preprocessor defines, namely W3MFC_EXT_CLASS, THRDPOOL_EXT_CLASS and SOCKMFC_EXT_CLASS. This allows the classes to easily be added and exported from a MFC extension dll.
V1.62 of W3MFC (3 March 2003)

Added a few defines to allow the W3MFC to compile if you do not have the latest version of the Platform SDK installed. Thanks to "Edgar" for reporting this.
Fixed a copy and paste gremlin in CHttpServer::Start which was causing a critical section to not be acquired. Thanks to "Edgar" for reporting this.
Removed the use of "friend classes" entirely throughout the W3MFC codebase. This avoids potential compilation problems in any derived classes used by client code of W3MFC.
Addition of a number of preprocessor defines, namely W3MFC_EXT_CLASS, THRDPOOL_EXT_CLASS and SOCKMFC_EXT_CLASS. This allows the classes to easily be added and exported from a MFC extension dll.
V1.61 of W3MFC (2 March 2003)

Removed the function CHttpDirectory::SetClient as it can lead to thread synchronisation problems. Thanks to "Edgar" for reporting this problem.
V1.60 of W3MFC (27 February 2003)

Includes ISAPI v5.1 Extension support (excluding the asynchronous extensions only supported by IIS itself)
Fixed a bug in the parsing of SSL requests which was causing heap corruption problems on subsequent reads of SSL requests on the same thread.
Reworked the CHttpClient::HandleClient to avoid having to call various functions when the code needs to exit prematurely
Added a member variable "m_bResponseKeepAlive" to avoid having to pass a keep alive variable throughout all the CHttpClient code.
Fixed a deadlock bug in CHttpServer::Stop. Thanks to "Edgar" for reporting this problem.
modified the Mime manager class method "GetMimeType" to be sent the full request rather than just the extension of the URL when it is called to determine the mime type.
Addition of a string version of the HTTP verb to CHttpRequest. This speeds up the CGI and ISAPI implementations somewhat.
Addition of a hash table in the request structure which contains all the HTTP request headers. These values are now available via CGI or ISAPI
Split of the CGI implementation into its own module and made it optional via a preprocessor directive
Split of the ISAPI implementation into its own module and made it optional via a preprocessor directive
W3MFC now uses Win32 TransmitFile API in SSL configuration and only falls back to user mode sockets if SSL is actively being used.
W3MFC now supports PATH_INFO information in URL's
Optimized loading of error pages which W3MFC uses
V1.56 of W3MFC (21 February 2003)

Made the m_Directories member of CHttpServerSettings a pointer array. This allows client code to add their own derived instances of CHttpDirectory to the array. This allows per directory customisation of the web server. This change also necessitated changing the settings class of the CHttpServer to be a pointer also. Thanks to "Edgar" for this update.
Remove the digest authentication boolean from the settings class as Digest authentication is currently not supported.
Made the CHttpClient::LoadHTMLResource method virtual
Moved a lot of the CHttpClient implementation code to CHttpDirectory. This allows more additional complex client customisation of the code.
Split off the CHttpDirectory class into its own module.
Implemented a virtual CHttpDirectory::HandleClient. This allows customisation of per directory responses. Thanks to "Edgar" for this suggestion.
V1.55 of W3MFC (3 February 2003)

W3MFC now compares HTTP headers without regard to case. Thanks to Gilad Novik and Frank Hahn for reporting these problems.
Tidied up inclusion of Afxpriv.h throughout W3MFC modules.
Fixed a few typos in the documentation.
25 November 2002

Added a comment to the HttpSocket.h header file about where to get the WTimer.h files if the W3MFC compile fails.
V1.54 of W3MFC (20 November 2002)

Fixed a number of level 4 warnings about unreferrenced variables. Thanks to Harold Johns for reporting these.
Updated the documentation to refer to the fact that you will need an up to date Platform SDK to be installed to compile W3MFC with SSPI support.
Fixed a memory leak of CHttpRequest objects which can occur when you shutdown the web server and you are using a directed thread pool queue and there are outstanding items in the queue which are not shutdown requests.
V1.52 of W3MFC (18 November 2002)

Now allows anonymous access to be enabled / disabled
Now allows Basic authentication to be enabled / disabled.
Reworked the Base64 class to be based on the ATL Server Base64 routines in ATL Server in VC.Net
Renamed and Reworked the CHttpClient::ReturnUnauthorizedMessage method.
Impersonation handle used during Basic authentication now is a CW32Handle instance instead of a raw SDK handle
Now fully supports NTLM Authentication via the SSPI interface. The code is enabled for support for Kerberos, "Negotiate" and Digest authentication which will be added in a future release. The use of SSPI support can be changed via the compile time define "W3MFC_NO_SSPI_SUPPORT". Thanks to Harold B Johns for suggesting this.
Fixed a problem in CHttpClient::ParseRequest which was causing failures to parse a certain line to be overwritten upon successful parsing of subsequent lines. 
Test CGI app now is implemented without MFC support and deliberately does not use a Content-Length or Keep-Alive header for testing purposes.
Tidied up the importing of various header files throughout W3MFC and also the associated
#pragma messages which these something display.
V1.53 of W3MFC (18 November 2002)

CHttpSocket::ReadResponse now uses waitable timers (if it can) instead of a loop which has calls to Sleep. This should improve the scalability of the code under heavy load. Please note that if you want to compile the code, then you will need to download the CWaitableTimer source code separately from my web site and copy in the wtimer.h & wtimer.cpp into your W3MFC's source code directory.
Added code to protect against NTFS alternate streams being accessed by client browsers.
Improved the robustness of parsing the authorization header fields.
V1.51 of W3MFC (17 November 2002)

Optimized the use of the Keep Alives variables in the function CHttpClient::HandleClient
Implemented code to find whether CGI programs send Keep Alives.
V1.50 of W3MFC (17 November 2002)

Class now supports the use of the "Connection: Keep-Alive" Header.
Removed the StringReplace method and replaced it with CString::Replace.
Fixed some Log message's text to correctly reflect the function they are used in.
Code now makes the call to CHttpClient::AllowThisConnection before the incoming request is parsed.
Fixed a typo in the name of the CHttpClient::HandleDirectoryAuthorization function
Made the operation of the delivery of the "Expires" header contingent on a configurable setting
CHttpResponseHeader::AddW3MfcAllowFields now returns that it can handle POST requests
The "DELETE" verb support is now contingent on a configurable setting.
Fixed a typo in the creation of the "SERVER_PROTOCOL" CGI environment variable
Updated the sample CGI app to also send a Content-Length HTTP header. This allows the sample app to work correctly when it is used in conjunction with HTTP Keep-Alives
V1.18 of CThreadPoolServer (10 November 2002)

Fixed an unreferrenced variable in the function CIOCPThreadPoolQueue::GetRequest, Thanks to Michael K. O'Neill for reporting this issue.
V1.17 of CThreadPoolServer (7 November 2002)

Minor update to the thread pool class to provide a virtual function which gets call when the m_bRequestToStop is being set.
V1.16 of CThreadPoolServer (17 October 2002)

Made the thread pool class Win95 compliant by dynamically linking to the waitable timer API's. Even though the code did gracefully degrade if the waitable timer functions failed, the fact that they did not use GetProcAddress to link to the functions meant that any app / dll which included the thread pool class would fail to load on Win95. Thanks to Frank Schmidt for this update.
V1.15 of CThreadPoolServer (17 October 2002)

Fixed a problem where CThreadPoolServer::Stop() would hang if an I/O completion port based thread pool is being used. Thanks to Frank Schmidt for spotting this problem.
V1.49 of W3MFC (17 October 2002)

Fixed a bug in CHttpClient::HexToInt which was causing it to not work for upper case characters. Thanks to Frank Schmidt for spotting this problem.
Failure to setup thread pool recycling is now reported as an error.
V1.14 of CThreadPoolServer (14 October 2002)

Reintroduced the function CThreadPoolQueue::PostRequestWithoutLimitCheck as some users of the thread pool class had legit reasons to use this function.
Changed a VERIFY call into an ASSERT in CThreadPoolServer::RecycleThread
V1.13 of CThreadPoolServer (12 October 2002)

Removed and replaced the PostRequestWithoutLimitCheck method with the standard PostRequest method. This avoids the problem with TRACE messages appearing along the lines "CDirectedThreadPoolQueue::GetRequest, Failed to release a semaphore". Thanks to Frank Schmidt for reporting this problem.
Fixed a minor flow in "CDirectedThreadPoolQueue::Create()" where I forgot to call Close() when the creation of "m_hGetRequestSemaphore" fails. Again thanks to Frank Schmidt for spotting this.
V1.12 of CThreadPoolServer (8 October 2002)

Shutting down of the thread pool now uses directed requests instead of undirected requests. This should improve the speed of shutdown of the thread pool when it contains a lot of requests
on the queue.
Defined enums for m_dwID member of the CThreadPoolRequest class
V1.11 of CThreadPoolServer (4 October 2002)

CThreadPoolClient::Run now has a return value to decide whether or not the worker thread should continue to service requests upon return from handling the current request
V1.10 of CThreadPoolServer (20 August 2002)

Provided virtual destructors for all the classes which constitute the Thread pool framework
Removed forward reference of the now defunct class CDirectedIOCP
Removed unreferenced parameters level 4 warnings in the CThreadPool class declaration
Fixed usage of 2 int variables in CDirectedThreadPoolQueue::GetNonDirectedRequestIndexToRemove and GetDirectedRequestIndexToRemove which were incorrectly declared as BOOL's. Thanks to Serhiy Pavlov for these updates.
V1.09 of CThreadPoolServer (18 August 2002)

Renamed CDirectedIOCP to CDirectedThreadPoolQueue
Renamed CDirectedIOCPRequest to CThreadPoolRequest
Now user can decide which queuing mechanism to use thro the Start method. 2 pre built classes are provided, namely CDirectedThreadPoolQueue and CIOCPThreadPoolQueue
Provision of virtual GetNonDirectedRequestIndexToRemove and GetDirectedRequestIndexToRemove methods in the CDirectedThreadPoolQueue class. This allows derived classes to implement their own schemes as to which requests to prioritize on the thread pool queue.
V1.48 of W3MFC (18 August 2002)

Now uses v1.09 of the CThreadPoolServer class. 
Sample app now allows you to decide via a ini value whether or not to use the CIOCPThreadPoolQueue class instead of the CDirectedThreadPoolQueue class.
V1.47 of W3MFC (28 July 2002)

Now CHttpMimeManager can be replaced at runtime. This allows the mime map to be easily configured or replaced with a custom implementation.
Mime map as used by the sample app is now configured from the W3MFC ini file instead of from the system registry. In addition the shipped W3MFC ini file now includes a fully fleshed out mime section
You can now improve the robustness of the SSL behaviour by initializing the OpenSSL PRNG (Pseudo Random Number generator) with the contents of a file as specified via CHttpServerSettings::m_sSSLRandomnessFile.
A fresh default web site is now shown using the default ini settings, instead of expecting a web site to be configured at c:\inetpub\wwwroot
Improved the performance of looking up mime types by implementing a hash table for storage
V1.46 of W3MFC (14 June 2002)

Fixed a problem in CHttpClient::FindNextTerminatorFromRequest which was parsing out header details incorrectly. Thanks to Gilad Novik for reporting this problem. 
4 June 2002

Minor update to fix a missing constructor for CW32Handle class.
V1.45 of W3MFC (30 May 2002)

Fixed a #include issue when doing a non SSL build. Thanks to Harold B Johns for spotting this.
Fixed an issue where a request for a non existent CGI file caused the web server to become unresponsive. Thanks to Harold B Johns for spotting this.
Fixed an issue in the CGI code when SSL is enabled. Thanks to Gilad Novik for reporting this problem.
V1.44 of W3MFC (29 May 2002)

Environment variables for CGI processes now includes all the environment variables which W3MFC itself has. These missing environment variables were causing CGI programs to fail in calls to CoInitialize if they were using COM. Thanks to Harold B Johns for spotting this.
V1.43 of W3MFC (25 May 2002)

Improved the performance of CHttpClient::URLDecode method
Improved the performance of CHttpClient::TransmitDirectory. Thanks to "Jim" for this fix.
URL's transmitted down to client in CHttpClient::TransmitDirectory are now URL encoded.
Fixed a crash which was occurring for some requests in CHttpClient::ParseRequest where the code ended up parsing into the entity body instead of stopping when it reached the separator after the http header. Thanks to Harold B Johns for spotting this.
V1.42 of W3MFC (24 May 2002)

Included info in the help file on how to use SSL in W3MFC
Fixed a bug in the SSL code path which was causing a crash in CHttpClient::TransmitBuffer.
Provision of wrapper classes for OpenSSL C type variables used by W3MFC
Tidied up the SSL code in CHttpClient::HandleClient.
V1.41 of W3MFC (23 May 2002)

Now fully supports SSL via the OpenSSL open source Library.
Removed IOCP9x module from project as not required.
Fixed a bug in CHttpServer::ListenSocketFunction which was causing the thread pool to initialize incorrectly.
V1.08 of CThreadPoolServer / V1.40 of W3MFC (29 April 2002)

Changed value in sample W3MFC.ini file to not do directory listing by default.
ThreadPool class now provides an option to have a Q size different than the thread pool size.
Also provides a method to post to the IOCP without first checking the limit.
V1.07 of CThreadPoolServer / V1.39 of W3MFC (29 April 2002)

Fixed a bug in the CDirectedIOCP class which was causing multiple threads in the thread pool to be released into the depths of the CDirectedIOCP::GetRequest code.
Fixed a bug which was causing m_Threads array to be accessed from multiple threads when thread recycling was enabled.
V1.06 of CThreadPoolServer / V1.38 of W3MFC (17 April 2002)

Move the sample app up to VC 6 instead of VC 5.
Thread Pool class now uses new "CDirectedIOCPRequest" class instead of an SDK IOCP or dummy one for Win9x. This allows the thread pool to now support recycling of threads after a specified interval in the thread pool.
Tidied up the exposed API of the thread pool class to not reflect the exact attributes of IO completion ports and instead be more object oriented.
Change some of the Thread Pool class API parameters to provide for more flexibility.
31 March 2002

Updated the sample app to print out the settings it is using when the program starts. No changes made to the core code.
V1.37 of W3MFC (23 March 2002)

W3MFC ships with a sample CGI app "TestCGI.exe"
CHttpClient::GetCGICommandLine function now defaults to the local file path if a registry entry cannot be found for it. Thanks to Gilad Novik for this suggestion.
V1.36 of W3MFC (21 February 2002)

Fixed an issue in CHttpClient::GetCGICommandLine where the code was using RegQueryValue instead of RegQueryValueEx which is the recommended way of doing registry queries in Win32.Thanks to Josh Clarke for spotting this problem.
CGI command line is now expanded with any environment variables it may contain. Again thanks goes to Josh Clarke for this nice update.
V1.01 of CIOCP9x / V1.35 of W3MFC (26 January 2002)

CIOCP9x class: added an additional semaphore as I have forgotten that we need to synchronise the removal of items from the queue in addition to synchronising the addition of items to the queue.
Fixed a level 4 warning in the CHttpClient class.
Only now uses TransmitPackets and TransmitFile if running 2000 / XP or .Net Server. Thanks to Olivier Meyer for spotting this problem on NT 4.
V1.34 of W3MFC / V1.0 of CW32Handle (8 January 2002)

Code only uses TransmitFile and TransmitPackets APIs if running on NT/2000/XP/.Net Server
Provided a virtual CHttpClient::TransmitFile and CHttpClient::TransmitBuffer functions. The default implementations just pass the buck to the _W3MFC_DATA function pointers if available.
Made CHttpClient::HandleClient function virtual.
Fixed an issue in CHttpClient::TransmitDirectory where created URL's could sometimes forget to put in directory separators in the URL.
Updated copyright messages to be 2002.
Provided a virtual CHttpClient::ParseRequestLine to allow derived classes to easily pull out any additional http headers e.g. "User-Agent" or "Accept-Language" etc.
Provided a virtual CHttpClient::FormCGIEnvironment to allow derived classes to easily change the environment variables sent to CGI processes.
Fixed a serious bug in CHttpResponseHeader::GetData while code was compiled for UNICODE
Addition of a very handy new "CW32Handle" class which implements a "Smart Pointer" class for Win32 Handles. For one thing it looks after calling CloseHandle in the destructor. The class turns out to be very handy when you are implementing CGI in this web server as you have to create loads of Win32 handles such as anonymous pipes etc.
Improved the error reporting in CHttpClient::TransmitCGIResponse
Rewrote the CGI handling code in CHttpClient::TransmitCGIResponse to use the information provided by MS Knowledge base article "Q190351".
V1.33 of W3MFC / V1.02 of CWSocket / V1.05 of CThreadPoolServer (27 December 2001)

Fixed an exception memory leak in the method CHttpSocket::SendWithRetry. Thanks to Olivier Meyer for spotting this problem.
Removed an unreferrenced variable from CHttpSocket::ReadResponse
Added a flag to TransmitFile to allow the transmission of an "Expires: " header
Added CHttpDirectory parameter to all transmit methods. Allows derived classes more control over the response procedure.
Added optional Non-NT authentication to the web server at the virtual directory level. 
Reviewed TRACE statements throughout W3MFC for completeness and validity
Addition of a CHttpClient::TransmitBuffer method to allow for easy transmission of in-memory generated responses. Thanks to "Golden Crater" for most of those suggestions.
W3MFC now takes advantage of the MS Winsock extension API "TransmitFile". This should improve the performance of the code by avoiding inordinate kernel transitions
on NT based operating systems.
Includes a new error reporting mechanism via the CHttpServer::OnError method
Now includes a nice icon for the generated binary
Version info resource is now included in the generated binary 
Modified "Return..." functions to return body length sent
Check all PostLog calls to ensure indicated parameter size is correct
W3MFC now takes advantage of the MS Winsock extension API "TransmitPackets". This should improve the performance of the code by avoiding inordinate kernel transitions on Windows XP or later. Please note that I have been unable to test this feature so feedback on this would be appreciated.
V1.32 of W3MFC (3 November 2001)

Fixed an issue where socket sends would sometimes throw the exception WSAEWOULDBLOCK. The httpsocket class now includes a "SendWithRetry" method which handles this case.
V1.01 of CWSocket / v1.31 of W3MFC (30 September 2001)

Fixed a bug in CWSocket::Receive which can occur when a disconnection occurs while a receive is in progress
Fixed a bug where entities sent i.e. m_pszRawEntity did not correctly handle the case where it had embedded NULL's e.g. a file upload. Thanks to Christian Hett for spotting this problem.
Removed some unnecessary memcpy's from CHttpClient::ParseRequest
Made the main program configurable via an ini file instead of hard coding all the settings.
Fixed a resource leak on an anonymous pipe handle in CHttpClient::TransmitCGIResponse.
V1.04 of CThreadPoolServer / v1.3 of W3MFC (26 August 2001)

Removed unnecessary "#pragma message" in thread pool implementation file
Added support for CGI v1.1 (has been verified with some perl scripts and batch files)
Added support for reverse DNS lookups
Now fully supports POST HTTP method.
Following values have been added to CHttpRequest:
m_pszRawRequest, m_pszRawEntity, m_sRemoteHost and m_sContentType
Improved the robustness of the code by making sure exceptions are caught
V1.03 of CThreadPoolServer / v1.23 of W3MFC (7 August 2001)

Fixed some minor start-up problems with the sample web server app
Thread pool class now provides a WaitForThreadsInitInstance method. This allows client code to synchronise with all the InitInstance's in the thread pool.
V1.02 of CThreadPoolServer / v1.22 of W3MFC (25 July 2001)

Thread Pool Code now uses a Win9x compatible IO completion port if we fail to use the build in OS one. This IOCP is implemented in the class "CIOCP9x" in IOCP9x.h/cpp. This means that the web server now works on Windows 9x again.
W3MFC Code now use Winsock 2 functionality, namely "WSAEventSelect" to avoid a busy loop in the listening socket thread. This means that you will have to have Winsock 2 downloaded and installed if running on Windows 95 which did not ship with Winsock 2 built in.
V1.01 of CThreadPoolServer (21 July 2001)

Made destructors of the two classes virtual as both can be used as base classes.
V1.21 (23 April 2001)

Fixed a stack overwrite issue in CHttpResponseHeader::DateToStr
Fixed a bug in CHttpClient::TransmitFile which was causing "304 Not Modified" headers to never be returned.
Added a comment to documentation to inform users that the code will no longer work on Win 9x due to usage of IOCP.
V1.2 (15 April 2001)

Updated copyright information
A new generalised thread pool wrapper class is now included which can be used in other projects.
A new generalised sockets wrapper class is now included which can be used in other winsock projects.
V1.11 (29 February 2000)

Fixed a number of VC 6 level 4 warnings.
Now empties any passwords transmitted once they are used.
Now implements W3C Common Logfile logging to the console window
V1.1 (29 June 1999)

Implemented support for "HEAD" command
Sample provided now also displays the HTTP verb used
Sample provided now also displays the date and time of each request
Now fully supports multiple virtual directories
Now fully supports URL's with encoded characters
Implemented support for "DELETE" command
Now returns an "Allow:" HTTP header
Timeout for requests is now 90 seconds if built for debug
Now supports directory listing
User name is now displayed in the log window
V1.0 (4 May 1999)

Initial public release.
