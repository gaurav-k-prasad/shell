try:
  from dotenv import load_dotenv
  # from google import genai
  import argparse
  import time
  import json
  load_dotenv()
  parser = argparse.ArgumentParser()

  prompt: str = """You are an AI assistant for a Unix shell. Respond ONLY in JSON.

  User Input: create a new folder here

  Rules:
  1. If the input is a natural language instruction, output the exact shell command(s).  
  2. If a command is dangerous (e.g. deleting system files), include a warning.  
  3. Always explain briefly what the command(s) do.  
  4. If the input is unclear, incomplete, or not a valid instruction, ask clarifying question(s).  
  5. Do not output anything outside the specified JSON. 
  6. No use of emojis 
  7. If answer unknown do not guess give empty list of commands or questions and give explanation
  8. If multiple commands are required, list them in the order they must be executed.
  9. Always use absolute file paths

  JSON for Answering:
  {
    "commands": ["<command1>", "<command2>", ...],
    "explanation": "<short explanation in only 1 line>",
    "warning": "<warning message in only 1 line or empty string>"
  }

  JSON for Questioning:
  {
    "questions": ["<question1>", "<question2>", ...],
    "explanation": "<why these questions are needed in only 1 line>"
  }

  Examples:

  User Input: "Make a React app called my-app"  
  Output:
  {
    "commands": ["npx create-react-app my-app"],
    "explanation": "Creates a new React project called my-app.",
    "warning": ""
  }

  User Input: "Delete all files in /"  
  Output:
  {
    "commands": ["rm -rf /"],
    "explanation": "Deletes everything in the root directory.",
    "warning": "This will erase your system. Confirm before running."
  }

  User Input: "abcdfasdfasdf"  
  Output:
  {
    "questions": ["What would you like me to do?"],
    "explanation": "Unable to understand user query."
  }"""

  # client: genai.Client = genai.Client()

  # response = client.models.generate_content(
  #   model="gemini-2.5-flash", contents=prompt
  # )

  # print(response.text)
  # exit(0)

  a = """```json
  {
    "commands": [
      "ls",
      "ls > write.txt"
    ],
    "explanation": "These commands install PyTorch (with CUDA 12.1 build, which is compatible with CUDA 12.4) and the latest stable version of TensorFlow. TensorFlow automatically detects and utilizes your system's CUDA installation.",
    "warning": "Ensure that the NVIDIA CUDA Toolkit 12.4 and a compatible cuDNN library are already installed and properly configured on your system for these installations to fully leverage your GPU. If you are using a virtual environment, activate it before running these commands."
  }"""

  result = a
  # if (response.text and response.text.startswith("```")):
  #   result = response.text.strip("```json").strip()
  if (a and a.startswith("```")):
    result = a.strip("```json").strip()

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

  time.sleep(0.5)
  if (result):
    resJson = dict(json.loads(result))

    if ("commands" in resJson):
      with open("ai_output.txt", "w") as f:
        writeSection("OUTPUT", "command", f)
        writeSection("EXPLANATION", resJson.get("explanation"), f)
        writeSection("WARNING", resJson.get("warning"), f)
        writeList("COMMANDS", resJson.get("commands", []), f)
    elif ("questions" in resJson):
      with open("ai_output.txt", "w") as f:
        writeSection("OUTPUT", "question", f)
        writeSection("EXPLANATION", resJson.get("explanation"), f)
        writeList("QUESTIONS", resJson.get("questions", []), f)
except:
  exit(1)