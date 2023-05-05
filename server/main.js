const express = require('express');
const { engine } = require('express-handlebars');
const app = express();

let newData = [];

app.engine('handlebars', engine());
app.set('view engine', 'handlebars');
app.set('views', './server/views');

app.get('/data', function (req, res) {
  if (newData.length < 1) {
    res.send('No new data!');
  } else {
    res.send(newData + '\n');
  }
});

app.get('*', (req, res) => {
  res.render('home', {
    data: newData
  })
})

app.listen(3000);

process.stdin.on('data', (data) => {
  console.log(`stdin ${data.toString()}`);

  newData = data;
});
