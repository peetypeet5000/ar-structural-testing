const process = require('node:child_process');

const express = require('express');
const { engine } = require('express-handlebars');
const app = express();

let newData = '';

app.engine('handlebars', engine());
app.set('view engine', 'handlebars');
app.set('views', './server/views');

/*
* Route: GET /data
* Description: Sends formatted data back as text/html
*/
app.get('/data', function (req, res) {
  if (newData.length < 1) {
    res.send('No new data!');
  } else {
    // Get only the last line of data and send with appended newline
    let data = newData.slice(0, newData.indexOf('\n'));
    res.send(data + '\n');
  }
});

/*
* Route: GET any remaining route
* Description: Sends data in handlebars home template for easier human reading
*/
app.get('*', (req, res) => {
  // Format the data into an array and pop off the last newline
  let dataArray = newData.split('\n');
  if(dataArray[dataArray.length - 1] === '') {
    dataArray.pop();
  }

  res.render('home', {
    data: dataArray
  })
})


// Spawns the serial reader as a child process and listens to its stdout
var child = process.spawn('./serial/serial_reader');

child.stdout.on('data', function (data) {
  newData = data.toString();
});

// Starts the webserver, listening on this computers IP to local network
app.listen(3000, '0.0.0.0');

