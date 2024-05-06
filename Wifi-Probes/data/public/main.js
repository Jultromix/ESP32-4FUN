// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);


// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      
      document.getElementById("humsensor1").innerHTML =  myObj.humsensor1;
      document.getElementById("humsensor2").innerHTML =  myObj.humsensor2;
      document.getElementById("humsensor3").innerHTML =  myObj.humsensor3;
      document.getElementById("tempsensor1").innerHTML =  myObj.tempsensor1;
      document.getElementById("tempsensor2").innerHTML =  myObj.tempsensor2;
      document.getElementById("tempsensor3").innerHTML =  myObj.tempsensor3;
      document.getElementById("tempsensor4").innerHTML =  myObj.tempsensor4;
      document.getElementById("gassensor1").innerHTML =  myObj.gassensor1;
      document.getElementById("gassensor2").innerHTML =  myObj.gassensor2;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

// Create a new EventSource object and specify the URL of the page sending the updates. In our case, it’s /events.
if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    document.getElementById("humsensor1").innerHTML =  myObj.humsensor1;
    document.getElementById("humsensor2").innerHTML =  myObj.humsensor2;
    document.getElementById("humsensor3").innerHTML =  myObj.humsensor3;
    document.getElementById("tempsensor1").innerHTML =  myObj.tempsensor1;
    document.getElementById("tempsensor2").innerHTML =  myObj.tempsensor2;
    document.getElementById("tempsensor3").innerHTML =  myObj.tempsensor3;
    document.getElementById("tempsensor4").innerHTML =  myObj.tempsensor4;
    document.getElementById("gassensor1").innerHTML =  myObj.gassensor1;
    document.getElementById("gassensor2").innerHTML =  myObj.gassensor2;
  }, false);
}