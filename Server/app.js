const express = require('express');
const cors = require('cors');

const app = express();

app.use(cors());
app.options('*', cors());

app.get('/hello', (req, res) => {
  // Return some sample data as the response
  res.json({
    message: 'Hello, world!'
  });
  console.log('Hello World');
});

app.get('/python', (req, res) => {
  // Return some sample data as the response
  res.json({
    message: 'Hello, from python script!'
  });
  console.log('Hello, from python script!');
});


module.exports = app;
