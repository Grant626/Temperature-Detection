const express = require('express');
const cors = require('cors');
const path = require('path');
const bodyParser = require('body-parser');
const fs = require('fs');
const fileName = './public/test.json';
const file = require(fileName);


const app = express();

app.use(cors());
app.use(express.static('public'));
app.use(bodyParser.urlencoded({ extended: false }));
app.options('*', cors());

app.set('views', path.join(__dirname, 'views'))
app.set('view engine', 'ejs')

app.get('/', (req, res) => {
  // Return some sample data as the response
  console.log(file)
  res.render('test', {
    maxTemp: file.maxTemp
  });
  
});


app.post('/', (req, res) => {
  const { maxTemp } = req.body
  console.log(maxTemp);

  file.maxTemp = maxTemp;
    
  fs.writeFile(fileName, JSON.stringify(file), function writeJSON(err) {
    if (err) return console.log(err);
    console.log(JSON.stringify(file));
    console.log('writing to ' + fileName);
  });

  location.reload();
  res.status(200).redirect('/')
});


module.exports = app;
