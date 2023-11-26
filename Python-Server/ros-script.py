import requests
import sys

node = sys.argv[1]
api = "http://127.0.0.1:8000/"

def saveNodeFrame(node):
  response = requests.post(api + "save_frame" + "?node=%s" % node)
  if response.status_code == 200:
    print("Successfully posted frame for node: %s" % node)
  else:
    print("Error (%s) while posting frame for node: %s" % (response.status_code, node))
    
if __name__ == "__main__":
  saveNodeFrame(node)