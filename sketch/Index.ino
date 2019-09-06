void HTML_INDEX()
{
//   if (!server.authenticate(www_username, www_password)) {
//      return server.requestAuthentication();
//    }
  String Str = "<!DOCTYPE html>\n\
          <html lang=\"ru\">\n\
          <head>\n\
            <meta charset=\"utf-8\">\n\
            <title>Dose.IO</title>\n\
          </head>\n\
            <body>\n\
              <h1>Please use the application</h1>\n\
              <div id=\"welcome\"></div>\n\
              <script>\n\
              </script>\n\
            </body>\n\
          </html>";
  server.send(200, "text/html", Str);
}


