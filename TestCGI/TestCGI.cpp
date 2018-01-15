#include <stdio.h>
#include <stdlib.h>

int main()
{
  //Send the header
  //printf("HTTP/1.0 200 OK\n");
  printf("Content-Type: text/html\n\n");

  //Print the body
  printf("<html><head><title>Environment variables passed to this CGI App from W3MFC</title></head><body>");

  //Add all the environment variables
  for (char** szVariable = _environ; *szVariable; szVariable++)
    printf("%s<br>\n", *szVariable);

   //Used for testing the CGI parsing bug found in W3MFC v1.72
  //printf("datadatadatadatadatadataHTTP/1.0 200 OK\n");
  //printf("datadatadatadatadatadatadatadatadataHTTP/1.0 200 OK\n");

  //Print the end of the body
  printf("</body></html>");

  return 0;
}