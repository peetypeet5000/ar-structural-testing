const express = require('express')
const app = express()

let newData = [];

app.get('/data', function (req, res) {
  if(newData.length < 1) {
    res.send('No new data!');
  } else {
    res.send(newData + '\n')
  }

})

app.listen(3000)


process.stdin.on('data', data => {
  console.log(`stdin ${data.toString()}`);

  newData = data;
});