const process = require('node:child_process');

const express = require('express');
const { engine } = require('express-handlebars');
const app = express();

let newData = '';

app.engine('handlebars', engine());
app.set('view engine', 'handlebars');
app.set('views', './server/views');

app.get('/data', function (req, res) {
  if (newData.length < 1) {
    res.send('No new data!');
  } else {
    let data = newData.slice(0, newData.indexOf('\n'));
    res.send(data + '\n');
  }
});

app.get('*', (req, res) => {
  res.render('home', {
    data: newData
  })
})


var child = process.spawn('./src/main_current_test');

child.stdout.on('data', function (data) {
  newData = data.toString();
});

app.listen(3000, '0.0.0.0');

