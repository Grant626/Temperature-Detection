import requests
import sys

node = sys.argv[1]
api = "http://127.0.0.1:8000/"

def saveNodeFrame(node):
  # Request to save frame to server directory
  response = requests.post(api + "save_frame" + "?node=%s" % node)
  
  # Requests to save node data to json database
  requests.post(api + "node_temp" + "?node=%s" % node)
  requests.post(api + "node_status" + "?node=%s" % node)
  requests.post(api + "node_checked" + "?node=%s" % node)
  
  if response.status_code == 200:
    print("Successfully posted frame for node: %s" % node)
  else:
    print("Error (%s) while posting frame for node: %s" % (response.status_code, node))

if __name__ == "__main__":
  saveNodeFrame(node)