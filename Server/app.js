const express = require('express');
const cors = require('cors');

const app = express();

app.use(cors());
app.options('*', cors());

app.get('/', (req, res) => {
  // Return some sample data as the response
  res.json({
    message: 'Hello, world!'
  });
  console.log('Hello World');
});

app.post('/python', (req, res) => {
  // Return some sample data as the response
  res.json({
    message: 'Hello, from python script!'
  });
  console.log(req);
});


module.exports = app;
