void HTML_INDEX()
{
//   if (!server.authenticate(www_username, www_password)) {
//      return server.requestAuthentication();
//    }
  String Str = "<!DOCTYPE html>\n\
              <html>\n\
                <head>\n\
                  <meta charset=\"utf-8\">\n\
                  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=1\">\n\
                  <title>ESP8266</title>\n\
                  <link href=\"\" rel=\"stylesheet\">\n\
                  <style>#navbar ul{display: none;background-color: #f90;position: absolute;top: 100%;}#navbar li:hover ul { display: block; }#navbar, #navbar ul{margin: 0;padding: 0;list-style-type: none;}#navbar {height: 30px;background-color: #666;padding-left: 25px;min-width: 470px;}#navbar li {float: left;position: relative;height: 100%;}#navbar li a {display: block;padding: 6px;width: 100px;color: #fff;text-decoration: none;text-align: center;}#navbar ul li { float: none; }#navbar li:hover { background-color: #f90; }#navbar ul li:hover { background-color: #666; }</style>\n\
                  <script type=\"text/javascript\">var xmlHttp=createXmlHttpObject();\n function createXmlHttpObject(){if(window.XMLHttpRequest){xmlHttp=new XMLHttpRequest();}else{xmlHttp=new ActiveXObject('Microsoft.XMLHTTP')}return xmlHttp;}\n function process () { if(xmlHttp.readyState ==0 || xmlHttp.readyState ==4){handleServerResponse();\n xmlHttp.open(\'GET\',\'xml\',true);\n xmlHttp.overrideMimeType(\'text/plain\');\nxmlHttp.send(null);\n}\n}setTimeout('process()',1000);\n  function Patch(DATA){var cache = {};\n var cacheData = cache['Folder.jpg'];\n if (cacheData) {document.location.replace(\"http://"+host+"/icon\");};\n  var patch=DATA.split('/');\n var p = document.getElementById('container');\nvar div = document.createElement('DIV');\n p.appendChild(div);\n div.appendChild(document.createTextNode('Путь: '));\nfor(var i=1;i<patch.length;i++){\ndiv.appendChild(document.createTextNode('/'));\n var e= document.createElement('a');\n  e.title =patch[i];\n e.href = \"http://"+host+"/folder?p=\"+patch[i]+\":\"+(i-1);\n e.appendChild(document.createTextNode(patch[i]));\n div.appendChild(e);\n}}\n function add_folder(DATA) {var arr=DATA.split('|',2);\n Patch(arr[1]);\n var arr1=arr[0].split(')',2);\nvar arr2=arr1[1].split(';',arr1[0]);\nfor(var i=0;i<arr1[0];i++)\n{var p = document.getElementById(\"folderi\") ;\nvar img = document.createElement(\"IMG\");\n img.width=50;\n img.height=50;\n  var CheckNew = document.createElement('input');\n var l = document.createElement('label');\n CheckNew.type = \"checkbox\";\n l.appendChild(document.createTextNode(\"\t\"));\n l.appendChild(CheckNew);\n l.appendChild(document.createTextNode(\"Первый\"));\n if(arr2[i].split(':',2)[0]=='D')img.src = \"http://"+host+"/Folder.jpg\";\n if(arr2[i].split(':',2)[0]=='F')img.src = \"http://"+host+"/File.jpg\";\n var div = document.createElement('div');\n var e= document.createElement('a');\n e.appendChild(img);\n e.title =arr2[i].split(':',2)[1];\n e.href = \"http://"+host+"/folder?f=\"+arr2[i]+\":\"+i;\n e.appendChild(document.createTextNode(arr2[i].split(':',2)[1]));\n p.appendChild(div);\n div.appendChild(e);\n /*div.appendChild(l);\n*/}};\n function handleServerResponse(){if(xmlHttp.status==200){xmlResponse=xmlHttp.responseText;add_folder(xmlResponse);}}\n </script>\n\
                </head>\n\
                <body onload=\"process();\">\n\
                  <ul id=\"navbar\">\n\
                    <li><a href=\"http://"+host+"/index\">Главная</a></li>\n\
                    <li><a href=\"http://"+host+"/graph\">Графики</a></li>\n\
                    <li><a href=\"#3\">Таблица</a>\n\
                      <ul>\n\
                        <li><a href=\"http://"+host+"/table\">Текущий</a></li>\n\
                        <li><a href=\"#32\">Телефон</a></li>\n\
                        <li><a href=\"#33\">Email</a></li>\n\
                      </ul>\n\
                    </li>\n\
                    <li><a href=\"http://"+host+"/wifi\">WIFI</a></li>\n\
                  </ul>\n\
                  <div id='container'></div>\n\
                  <div id='patch1'></div>\n\
                  <div id='patch2'></div>\n\
                  <form><div id=\"folderi\"><A HREF=\"http://"+host+"/folder?f=Log\"></A></div></form>\n\
               </body>\n\
            </html>";
  server.send(200, "text/html", Str);
}


