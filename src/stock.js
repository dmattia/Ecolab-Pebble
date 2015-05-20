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
  var url = "https://www.kimonolabs.com/api/9occjwra?apikey=FS663r7fk01mUFMbpVQbEshFrX90B5N7";

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with stock info
      var json = JSON.parse(responseText);

      // Get stock price
			var price = json.results.collection1[0].property1;
      console.log("Stock price is " + price);
			
			// Get stock change
			var changeFloat = parseFloat(price) - parseFloat(json.results.collection2[0].prev_close);
			var change = json.results.collection1[1].property1;
			var isNegative = 0;
			console.log("Stock change is " + changeFloat);
			if(changeFloat < 0) {
				isNegative = 1;
			}
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_PRICE": price,
				"KEY_CHANGE": change,
				"KEY_NEGATIVE": isNegative
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