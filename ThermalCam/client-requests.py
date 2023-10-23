import requests

url = 'http://localhost:3000/'

payload = {
    'message': 'Detected temperature above 30C!!!'
}

requests.post(url, data=payload)
