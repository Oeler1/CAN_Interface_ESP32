/*Dieser Header verwendet HTML, CSS Styling und javascript.
  Klassen (class) wird für das Einstellen von Styles benutzt, heißt in den Klassen ist definiert wie die Webseite visuell designed ist. 
  ids werden für die Zuweisung von Daten verwendet in der html und javascript Umgebung.
  Der xml code füllt die ids mit Daten, die dann anschließend vom javascript mit getElementById ausgelesen und verarbeitet werden.
  Generell sind die Hauptsektionen eines Webseitencodes so aufgebaut.

  <html>
    <style>
    // definieren von Klassen(CSS) für das gewünschte Aussehen
    </style>
    <body>
      <header>
      // Überschrift der Webseite
      </header>
      <main>
      // Aufbau der Webseite
      </main>
      <footer>
      // Fußnote
      </footer>
    </body>
    <script>
    // Hier werden dann tatsächlich Funktionen ausgeführt und Daten bearbeitet.
    </script>
  </html>


*/
// note R"KEYWORD( html page code )KEYWORD"; 


 const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">

<title>Web Page Update Demo</title>
  <style>
    table 
    {
      position: relative;
      width:100%;
      border-spacing: 0px;
    }

    tr 
    {
      border: 1px solid white;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
    }

    th 
    {
      height: 20px;
      padding: 3px 15px;
      background-color: #343a40;
      color: #FFFFFF !important;
    }

    td 
    {
      height: 20px;
      padding: 3px 15px;
    }

    .tabledata 
    {
      font-size: 24px;
      position: relative;
      padding-left: 5px;
      padding-top: 5px;
      height:   25px;
      border-radius: 5px;
      color: #FFFFFF;
      line-height: 20px;
      transition: all 200ms ease-in-out;
      background-color: #00AA00;
    }

    .messagebox 
    {
      width: 30%;
      height: 55px;
      outline: none;
      height: 25px;
      font-size: 20px;
    }

    .bodytext 
    {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 24px;
      text-align: left;
      font-weight: light;
      border-radius: 5px;
      display:inline;
    }

    .navbar 
    {
      width: 100%;
      height: 50px;
      margin: 0;
      padding: 10px 0px;
      background-color: #FFF;
      color: #000000;
      border-bottom: 5px solid #293578;
    }

    .fixed-top 
    {
      position: fixed;
      top: 0;
      right: 0;
      left: 0;
      z-index: 1030;
    }

    .navtitle 
    {
      float: left;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 50px;
      font-weight: bold;
      line-height: 50px;
      padding-left: 20px;
    }

   .navheading 
   {
     position: fixed;
     left: 60%;
     height: 50px;
     font-family: "Verdana", "Arial", sans-serif;
     font-size: 20px;
     font-weight: bold;
     line-height: 20px;
     padding-right: 20px;
   }

   .navdata 
   {
      justify-content: flex-end;
      position: fixed;
      left: 70%;
      height: 50px;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      font-weight: bold;
      line-height: 20px;
      padding-right: 20px;
   }

    .category 
    {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: bold;
      font-size: 32px;
      line-height: 50px;
      padding: 20px 10px 0px 10px;
      color: #000000;
    }

    .heading 
    {
      font-family: "Verdana", "Arial", sans-serif;
      font-weight: normal;
      font-size: 28px;
      text-align: left;
    }
  
    .btn 
    {
      background-color: #444444;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 4px 2px;
      cursor: pointer;
    }

    .foot 
    {
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
      position: relative;
      height:   30px;
      text-align: center;   
      color: #AAAAAA;
      line-height: 20px;
    }

    .container 
    {
      max-width: 1800px;
      margin: 0 auto;
    }

    table tr:first-child th:first-child 
    {
      border-top-left-radius: 5px;
    }
    table tr:first-child th:last-child 
    {
      border-top-right-radius: 5px;
    }
    table tr:last-child td:first-child 
    {
      border-bottom-left-radius: 5px;
    }
    table tr:last-child td:last-child 
    {
      border-bottom-right-radius: 5px;
    }
  </style>

  <body style="background-color: #efefef" onload="process()">  
    <header>
      <div class="navbar fixed-top">
          <div class="container">
            <div class="navtitle">ESP32 CAN</div>
            <div class="navdata" id = "date">dd.mm.yyyy</div>
            <div class="navheading">Datum</div><br>
            <div class="navdata" id = "time" >00:00:00</div>
            <div class="navheading">Uhrzeit </div>
            
          </div>
      </div>
    </header>
  
    <main class="container" style="margin-top:70px">
      <div class="category">Sensoren</div>
      <div style="border-radius: 10px !important;">
      <table style="width:50%">
      <colgroup>
        <col span="1" style="background-color:rgb(230,230,230); width: 20%; color:#000000 ;">
        <col span="1" style="background-color:rgb(200,200,200); width: 15%; color:#000000 ;">
        <col span="1" style="background-color:rgb(180,180,180); width: 15%; color:#000000 ;">
      </colgroup>
      <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
      <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
      <col span="2"style="background-color:rgb(0,0,0); color:#FFFFFF">
      <tr>
        <th colspan="1"><div class="heading">Pin</div></th>
        <th colspan="1"><div class="heading">Bits</div></th>
        <th colspan="1"><div class="heading">Volts</div></th>
      </tr>
      <tr>
        <td><div class="bodytext">Analog pin 7</div></td>
        <td><div class="tabledata" id = "b0"></div></td>
        <td><div class="tabledata" id = "v0"></div></td>
      </tr>
      <tr>
        <td><div class="bodytext">Analog pin 12</div></td>
        <td><div class="tabledata" id = "b1"></div></td>
        <td><div class="tabledata" id = "v1"></div></td>
      </table>
    </div>
    <br>
    <div class="category">Controls</div>
    <br>
    <div class="bodytext">RESET TWAI Controller </div>
    <button type="button" class = "btn" id = "btn0" onclick="ButtonPress0()">Toggle</button>  <!--Knopf für das Resetten des TWAI Controllers-->
    <br>
    <br>
    
    <br>
	  <input type="text" class="messagebox" id = CANID placeholder = "Hier CAN-ID eingeben" width = "0%" /> <!--Eingabetextfenster für CAN-ID-->
	  <br>
    <br>
    <input type="text" class="messagebox" id = Botschaft placeholder = "Hier CAN-Botschaft eingeben" width = "0%" /> <!--Eingabetextfenster für CAN Botschaft-->
    <button type="button" id = "btn2" onclick="Eingabe()">Senden</button> <!--Knopf für das Starten der Eingabe() Funktion-->
	  <br>
    <br>

    <br>
  </main>

  <footer div class="foot" id = "temp" >ESP32 CAN-Communication Web Page </div></footer>
  
  </body>


  <script type = "text/javascript">
    //Die Funktionalitäten der Webseite werden hier als Javascript geschrieben

    // global variable visible to all java functions
    var xmlHttp=createXmlHttpObject();

    // function to create XML object
    function createXmlHttpObject()
    {
      if(window.XMLHttpRequest){
        xmlHttp=new XMLHttpRequest();
      }
      else
      {
        xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
      }
      return xmlHttp;
    }

    // function to handle the button press from HTML code above
    // and send a processing string back to server
    // this processing string is use in the .on method
    function ButtonPress0() 
    {
      var xhttp = new XMLHttpRequest(); 
      var message;
       
      xhttp.open("PUT", "BUTTON_0", false);
      xhttp.send();
    }
    // Funktion für das Einlesen von CAN-ID und CAN Botschaft
    // Gibt die Daten ebenfalls an die log Konsole im Browser(F12) aus    
    function Eingabe() 
    {
      var xhttp = new XMLHttpRequest();
      var xhttp1 = new XMLHttpRequest();
      var dt = new Date();

	    var identification = document.getElementById("CANID").value //Speichern von CAN-ID eingabe aus HTML
      var betrag = document.getElementById("Botschaft").value //Speichern von CAN-Botschaft eingabe aus HTML
      var datetime = dt.toLocaleTimeString();

      // xhttp.open start über server.on die Eingabe_ID Funktion mit dem Argument IDENTIFICATION und dem Wert von identification
      // Das Gleiche für EINGABE_Botschaft
      // Es wird am Ende Timestamp, ID und Botschaft an die Konsole geschickt
      
      xhttp.open("PUT","EINGABE_ID?IDENTIFICATION="+identification, true);
      xhttp1.open("PUT","EINGABE_Botschaft?VALUE="+betrag, true);
      xhttp.send();
      xhttp1.send();
      console.log('GESENDET: ');
      console.log('Gesendete Uhrzeit: ', datetime);      
      console.log('Gesendete ID: ', identification);
      console.log('Gesendete Botschaft: ', betrag); 
    }

    // function to handle the response from the ESP
    function response()
    {
      var message;
      var barwidth;
      var currentsensor;
      var xmlResponse;
      var xmldoc;
      var dt = new Date();
      var datetime
      var color = "#e8e8e8";
     
      // get the xml stream
      xmlResponse=xmlHttp.responseXML;
  
      // get host date and time
      document.getElementById("time").innerHTML = dt.toLocaleTimeString(); //Zeit für Header in HTML
      document.getElementById("date").innerHTML = dt.toLocaleDateString(); //Datum für Header in HTML
      
      //Gesendete Informationen

      // ID Data
      xmldoc = xmlResponse.getElementsByTagName("ID0"); // ID in xmldoc speichern
      message = xmldoc[0].firstChild.nodeValue;         // Wert in message gespeichert

      // Botschaft Data
      xmldoc = xmlResponse.getElementsByTagName("BO0"); // Botschaft in xmldoc speichern
      message = xmldoc[0].firstChild.nodeValue;         //Wert in message gespeichert

      //Erhaltene CAN-ID
      xmldoc = xmlResponse.getElementsByTagName("FL1"); //Receive Flag in xmldoc speichern
      var flag = xmldoc[0].firstChild.nodeValue;        //In var Flag speichern

      message = xmlResponse.getElementsByTagName("ID1")[0].textContent;      // Erhaltene ID in message speichern
      var message2 = xmlResponse.getElementsByTagName("BO1")[0].textContent; //Erhaltene Botschaft in message2 speichern
      var datetime = dt.toLocaleTimeString();                                //Timestamp für die erhaltene Botschaft
      if (flag == 1)                                                         //Wenn Flag ==1 dann Ausgabe an Konsole
      {
       console.log('ERHALTEN: ');                       //Print der Uhrzeit, CAN-ID und Botschaft auf die Konsole
       console.log('Erhaltene Uhrzeit: ', datetime);
       console.log('Erhaltene ID: ', message);
       console.log('Erhaltene Botschaft: ', message2);
      }

      // ADC3V3
      xmldoc = xmlResponse.getElementsByTagName("B0"); //bits for ADC3V3
      message = xmldoc[0].firstChild.nodeValue;
      
      if (message > 2048)  //dynamische Farbe für Balken
      {
        color = "#aa0000";
      }
      else 
      {
        color = "#0000aa";
      }
      
      barwidth = message / 40.95;
      document.getElementById("b0").innerHTML=message;  //belegt b0 mit Wert für HTML
      document.getElementById("b0").style.width=(barwidth+"%");
      
      xmldoc = xmlResponse.getElementsByTagName("V0"); //volts for ADC3V3
      message = xmldoc[0].firstChild.nodeValue;
      document.getElementById("v0").innerHTML=message; //belegt v0 mit Wert für HTML
      document.getElementById("v0").style.width=(barwidth+"%");
      document.getElementById("v0").style.backgroundColor=color;
  
      // ADC5V
      xmldoc = xmlResponse.getElementsByTagName("B1"); //bits for ADC5V
      message = xmldoc[0].firstChild.nodeValue;
      if (message > 2048)
      {
        color = "#aa0000";
      }
      else 
      {
        color = "#0000aa";
      }
      document.getElementById("b1").innerHTML=message;
      width = message / 40.95;
      document.getElementById("b1").style.width=(width+"%");
      document.getElementById("b1").style.backgroundColor=color;
      
      xmldoc = xmlResponse.getElementsByTagName("V1"); //volts for ADC5V
      message = xmldoc[0].firstChild.nodeValue;
      document.getElementById("v1").innerHTML=message;
      document.getElementById("v1").style.width=(width+"%");
      document.getElementById("v1").style.backgroundColor=color;

      //BUTTON
      xmldoc = xmlResponse.getElementsByTagName("BUTTON");
      message = xmldoc[0].firstChild.nodeValue;
  
      if (message == 0)
      {
        document.getElementById("btn0").innerHTML="RESET";
      }
      else
      {
        document.getElementById("btn0").innerHTML="reset";
      }       


    }   
  
    // general processing code for the web page to ask for an XML stream
    // force this call with this for example
    // server.on("/xml", SendXML);
    // otherwise the page will not request XML from the ESP, and updates will not happen
    function process()
    {     
     if(xmlHttp.readyState==0 || xmlHttp.readyState==4) //readyState==0 heißt, dass client wurde erstellt aber noch nicht gesendet. readyState==4 Datentransfer erfolgreich beendet
     {
        xmlHttp.open("PUT","xml",true);
        xmlHttp.onreadystatechange=response;
        xmlHttp.send();
      }       
      // Eventuell mit verschiedenen Timeouts testen, wenn die erforderlichen Daten für zu groß werden und mehr Zeit brauchen
      setTimeout("process()",1000);
    }  
  
  </script>

</html>

)=====";
