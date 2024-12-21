#ifndef WEB_SERVER_FUNCTIONS_H
#define WEB_SERVER_FUNCTIONS_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Espalexa.h>

extern ESP8266WebServer server;
extern Espalexa espalexa;
extern bool motorRunning;
extern int rotationCount;
extern unsigned long timerDuration;
extern unsigned long timerStart;
extern int repeatCount;
extern bool repeatForever;
extern int feedCount;

void setupWebServer();
void handleRoot();
void handleFeed();
void handleStatus();
void handleSetTimer();
void handleCancelTimer();
void handleResetFeedCount();
void startMotor();
void stopMotor();

void setupWebServer() {
  server.on("/", HTTP_GET, [](){
    if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) {
      handleRoot();
    }
  });
  server.on("/feed", HTTP_GET, handleFeed);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/setTimer", HTTP_POST, handleSetTimer);
  server.on("/cancelTimer", HTTP_GET, handleCancelTimer);
  server.on("/resetFeedCount", HTTP_GET, handleResetFeedCount);
  
  server.onNotFound([](){
    if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0))) {
      server.send(404, "text/plain", "Not found");
    }
  });

  Serial.println("HTTP server configured");
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WiFi Cat Feeder</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
    h1 { color: #333; }
    button { font-size: 18px; padding: 10px 20px; margin: 10px; }
    #status, #countdown, #feedCount, #error { margin-top: 20px; }
    input { font-size: 18px; padding: 5px; margin: 5px; }
    label { font-size: 18px; }
    #error { color: red; }
  </style>
</head>
<body>
  <h1>WiFi Cat Feeder</h1>
  <button onclick="feedCat()">Feed Now</button>
  <div>
    <input type="number" id="hours" min="0" max="24" placeholder="Hours">
    <input type="number" id="repeats" min="0" placeholder="Repeat Count">
    <label><input type="checkbox" id="repeatForever"> Repeat Forever</label>
    <button onclick="setTimer()">Set Timer</button>
  </div>
  <button onclick="cancelTimer()">Cancel Timer</button>
  <p id="status"></p>
  <p id="countdown"></p>
  <p id="repeatInfo"></p>
  <p id="feedCount"></p>
  <button onclick="resetFeedCount()">Reset Feed Count</button>
  <p id="error"></p>
  <script>
    let countdownInterval;

    function showError(message) {
      document.getElementById('error').innerText = message;
    }

    function clearError() {
      document.getElementById('error').innerText = '';
    }

    function feedCat() {
      clearError();
      fetch('/feed')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          checkStatus();
        })
        .catch(error => {
          showError('Error: ' + error);
        });
    }

    function setTimer() {
      clearError();
      const hours = document.getElementById('hours').value;
      const repeats = document.getElementById('repeats').value;
      const repeatForever = document.getElementById('repeatForever').checked;
      
      fetch('/setTimer', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({hours: hours, repeats: repeats, repeatForever: repeatForever})
      })
      .then(response => response.text())
      .then(data => {
        document.getElementById('status').innerHTML = data;
        checkStatus();
      })
      .catch(error => {
        showError('Error setting timer: ' + error);
      });
    }

    function cancelTimer() {
      clearError();
      fetch('/cancelTimer')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          checkStatus();
        })
        .catch(error => {
          showError('Error cancelling timer: ' + error);
        });
    }

    function resetFeedCount() {
      clearError();
      fetch('/resetFeedCount')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerHTML = data;
          checkStatus();
        })
        .catch(error => {
          showError('Error resetting feed count: ' + error);
        });
    }

    function startCountdown(totalSeconds) {
      if (countdownInterval) {
        clearInterval(countdownInterval);
      }
      updateCountdown(totalSeconds);
      countdownInterval = setInterval(() => {
        totalSeconds--;
        if (totalSeconds <= 0) {
          clearInterval(countdownInterval);
          document.getElementById('countdown').innerHTML = "Feeding time!";
          setTimeout(checkStatus, 1000);
        } else {
          updateCountdown(totalSeconds);
        }
      }, 1000);
    }

    function updateCountdown(totalSeconds) {
      const hoursLeft = Math.floor(totalSeconds / 3600);
      const minutesLeft = Math.floor((totalSeconds % 3600) / 60);
      const secondsLeft = totalSeconds % 60;
      document.getElementById('countdown').innerHTML = `Next feed in: ${hoursLeft}h ${minutesLeft}m ${secondsLeft}s`;
    }

    function checkStatus() {
      clearError();
      fetch('/status')
        .then(response => response.text())
        .then(data => {
          const statusData = JSON.parse(data);
          document.getElementById('status').innerHTML = statusData.status;
          document.getElementById('feedCount').innerHTML = `Total feeds: ${statusData.feedCount}`;
          if (statusData.timerSeconds > 0) {
            startCountdown(statusData.timerSeconds);
            updateRepeatInfo(statusData.repeats, statusData.repeatForever);
          } else {
            clearInterval(countdownInterval);
            document.getElementById('countdown').innerHTML = "";
            document.getElementById('repeatInfo').innerHTML = "";
          }
        })
        .catch(error => {
          showError('Error checking status: ' + error);
        });
    }

    function updateRepeatInfo(repeats, repeatForever) {
      if (repeatForever) {
        document.getElementById('repeatInfo').innerHTML = "Timer will repeat indefinitely";
      } else if (repeats > 0) {
        document.getElementById('repeatInfo').innerHTML = `Timer will repeat ${repeats} more times`;
      } else {
        document.getElementById('repeatInfo').innerHTML = "";
      }
    }

    // Check status immediately when page loads
    checkStatus();
    // Then check status every 5 seconds
    setInterval(checkStatus, 5000);
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleFeed() {
  if (!motorRunning) {
    startMotor();
    server.send(200, "text/plain", "Cat is being fed!");
  } else {
    server.send(200, "text/plain", "Already feeding the cat!");
  }
}

void handleStatus() {
  DynamicJsonDocument doc(1024);
  
  if (motorRunning) {
    doc["status"] = "Feeding in progress";
  } else if (timerDuration > 0) {
    unsigned long remainingTime = (timerDuration - (millis() - timerStart)) / 1000;
    doc["status"] = "Timer running";
    doc["timerSeconds"] = remainingTime;
    doc["repeats"] = repeatCount;
    doc["repeatForever"] = repeatForever;
  } else {
    doc["status"] = "Ready to feed";
  }
  
  doc["feedCount"] = feedCount;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetTimer() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }
    
    int hours = doc["hours"];
    timerDuration = hours * 3600000; // Convert hours to milliseconds
    timerStart = millis();
    
    repeatForever = doc["repeatForever"];
    if (!repeatForever) {
      repeatCount = doc["repeats"];
    }
    
    char buffer[100];
    if (repeatForever) {
      snprintf(buffer, sizeof(buffer), "Timer set for %d hours, repeating indefinitely", hours);
    } else if (repeatCount > 0) {
      snprintf(buffer, sizeof(buffer), "Timer set for %d hours, repeating %d times", hours, repeatCount);
    } else {
      snprintf(buffer, sizeof(buffer), "Timer set for %d hours", hours);
    }
    server.send(200, "text/plain", buffer);
  } else {
    server.send(400, "text/plain", "Bad Request: No data received");
  }
}

void handleCancelTimer() {
  timerDuration = 0;
  repeatCount = 0;
  repeatForever = false;
  server.send(200, "text/plain", "Timer cancelled");
}

void handleResetFeedCount() {
  feedCount = 0;
  server.send(200, "text/plain", "Feed count reset to 0");
}

#endif // WEB_SERVER_FUNCTIONS_H
