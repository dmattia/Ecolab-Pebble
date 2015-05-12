var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getPrice() {
   // Construct URL
  var url = "http://www.google.com/finance/info?q=NSE:ECL";

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
			responseText = responseText.replace(/(\r\n|\n|\r)/gm,"");
			responseText = responseText.substring(4);
			responseText = responseText.substring(0,responseText.length-1);
      // responseText contains a JSON object with stock info
      var json = JSON.parse(responseText);

      // Get stock price
			var price = json.l;
      console.log("Stock price is " + price);
			
			// Get stock change
			var change = json.c;
			console.log("Stock change is " + change);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_PRICE": price,
				"KEY_CHANGE": change
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Stock info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getPrice();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getPrice();
  }                     
);