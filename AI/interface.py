def readFileContent(filename):
  content = []
  with open(filename, "r") as f:
    content = f.readlines()
  
  content = [contentItem.strip() for contentItem in content]
  res = ""
  for item in content:
    res += item + "\n"

  return res

def writeSection(section, text, filePtr):
    filePtr.write(f"==={section}===\n")
    if text:
      filePtr.write(text)
    filePtr.write("\n======\n")

def writeList(section, listItems, filePtr):
  filePtr.write(f"==={section}===\n")
  for item in listItems:
    filePtr.write(item + "\n")
  filePtr.write("======\n")

try:
  from dotenv import load_dotenv
  import requests, json, os, itertools, sys, argparse, threading, time
  load_dotenv()

  AI_OUTPUT_FILE = "gshellAIOutputInfo.txt"
  AI_ERROR_FILE = "gshellAIErrorInfo.txt"
  AI_HISTORY_FILE = "gshellAIHistoryInfo.txt"
  AI_PLATFORM_FILE = "gshellAIPlatformInfo.txt"

  parser = argparse.ArgumentParser(description="Shell AI Assistant")
  parser.add_argument("userInput", help="The natural language query or command")

  userInput = parser.parse_args().userInput

  historyInfo = readFileContent(AI_HISTORY_FILE)
  platformInfo = readFileContent(AI_PLATFORM_FILE)
  errorsInfo = readFileContent(AI_ERROR_FILE)

  prompt = f"""
You are an AI assistant for a Unix-like shell. 
Your ONLY output must be valid JSON, following one of the two schemas below.

User Query:
{userInput}

Command History:
{historyInfo}

User Context:
{platformInfo}

Errors (if any):
{errorsInfo}
""" + \
  """---
Rules:
1. Respond only in JSON, no extra text.
2. If the input is a natural language instruction, output the exact shell command(s).
3. If multiple commands are required, list them in execution order.
4. Always use the current working directory (CWD) when paths are needed.
5. Provide a brief, single-line explanation of what the commands do.
6. If the command is little dangerous (e.g., deletes system files), include a one-line warning.
7. If the input is unclear, incomplete, or invalid, ask clarifying question(s) instead of guessing.
8. Use command history and error info if relevant.
9. For "history" in the response, summarize both past history and the new command(s) in a few concise points.
10. No emojis or extra formatting outside JSON
11. If new line(\\n) required always use \\\\n for excape sequence etc and echo with -e flag to write to a file.
12. (<cmd1>;<cmd2>)2> >(tee errorInfo.txt) would be the format that the commands would be executed in bash. and the errorInfo.txt would be reevaulated if errors
13. If Explanation of something use Answering Schmea, keep commands list empty
---
JSON Schema for Answering:
{
  "commands": ["<command1>", "<command2>", ...],
  "explanation": "<short explanation in 1 line>",
  "warning": "<warning in 1 line or empty string>",
  "history": ["<summary of history + new command>"]
}
JSON Schema for Questioning:
{
  "questions": ["<clarifying question1>", "<clarifying question2>", ...],
  "explanation": "<why clarification is needed in 1 line>"
}
---
Examples
User Query: "Make a React app called my-app"
Output:
{
  "commands": ["npx create-react-app my-app"],
  "explanation": "Creates a new React project called my-app.",
  "warning": "",
  "history": ["Created React app my-app"]
}

User Query: "Delete all files in /"
Output:
{
  "commands": ["rm -rf /"],
  "explanation": "Deletes everything in the root directory.",
  "warning": "This will erase your system. Confirm before running.",
  "history": ["Attempted deletion of root directory"]
}

User Query: "abcdfasdf"
Output:
{
  "questions": ["What would you like me to do?"],
  "explanation": "Unable to understand the user query."
}
"""

  API_KEY = os.getenv("GEMINI_API_KEY")
  ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent"

  headers = {
    "Content-Type": "application/json",
    "x-goog-api-key": f"{API_KEY}"
  }

  data = {
    "contents": [
      {
        "parts": [
          {"text": prompt}
        ]
      }
    ]
  }

  # Spinner thread function
  def spinner_task(stop_event):
    spinner = itertools.cycle(["|", "/", "-", "\\"])
    while not stop_event.is_set():
      sys.stdout.write(next(spinner))  # write the next character
      sys.stdout.flush()
      time.sleep(0.1)
      sys.stdout.write("\r")
      sys.stdout.write("\033[2K")  # backspace to overwrite

  stop_event = threading.Event()
  spinner_thread = threading.Thread(target=spinner_task, args=(stop_event,))
  spinner_thread.start()

  try:
    resp = requests.post(ENDPOINT, headers=headers, json=data)
  finally:
    stop_event.set()   # stop spinner
    spinner_thread.join()

  response = None

  if resp.status_code == 200:
    response = resp.json()["candidates"][0]["content"]["parts"][0]["text"]
    print(response)
  else:
    print("Error:", resp.status_code, resp.text)
    raise Exception("AI Call failed")

  if (response and response.startswith("```")):
    response = response.strip("```json").strip()
  print(response)
  shellTest = """```json
  {
  "commands": [
    "ls",
    "lpic"
  ],
  "explanation": "These commands install PyTorch (with CUDA 12.1 build, which is compatible with CUDA 12.4) and the latest stable version of TensorFlow. TensorFlow automatically detects and utilizes your system's CUDA installation.",
  "warning": "Ensure that the NVIDIA CUDA Toolkit 12.4 and a compatible cuDNN library are already installed and properly configured on your system for these installations to fully leverage your GPU. If you are using a virtual environment, activate it before running these commands."
  }"""
  """
  result = shellTest
  if (shellTest and shellTest.startswith("```")):
    result = shellTest.strip("```json").strip()
  """
  
  if (response):
    resJson = dict(json.loads(response))

    if ("commands" in resJson):
      with open(AI_OUTPUT_FILE, "w") as f:
        writeSection("OUTPUT", "command", f)
        writeSection("EXPLANATION", resJson.get("explanation"), f)
        writeSection("WARNING", resJson.get("warning"), f)
        writeList("COMMANDS", resJson.get("commands", []), f)
      with open(AI_HISTORY_FILE, "w") as f:
        for item in resJson.get("history", []):
          f.write(item + "\n")
    elif ("questions" in resJson):
      with open(AI_OUTPUT_FILE, "w") as f:
        writeSection("OUTPUT", "question", f)
        writeSection("EXPLANATION", resJson.get("explanation"), f)
        writeList("QUESTIONS", resJson.get("questions", []), f)
except Exception as e:
  print(e)
  exit(1)