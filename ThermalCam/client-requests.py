import requests

url = 'http://localhost:3000/python'

correct_payload = {
    'message': 'Detected temperature above 30C!!!'
}

# Output => OK
r = requests.post(url, data=correct_payload)
print(r.text)
