import json
import os
import subprocess

subprocess.run("make")
test_cases_amn = int(len(os.listdir("testing/files")) / 2)

for i in range(test_cases_amn):
    process = subprocess.run(f"./parse testing/files/{i}_code.txt", shell=True, capture_output=True, text=True)
    output = process.stdout
    return_code = process.returncode

    with open(f"testing/files/{i}_result.json") as file:
        json_string = file.read()
        desired = json.loads(json_string)

    print(f"Testing {i}: {desired["description"]}")

    assert str(return_code) == desired["return_code"], f"Test Case {i} failed. Return code should be {desired["return_code"]}, but is {return_code}.\nOutput:\n{output}"
    assert desired["output"] in output, f"Test Case {i} failed. Desired output not in output.\nDesired:\n{desired["output"]}\nOutput:\n{output}"

print(f"Tests passed! {test_cases_amn} total test cases.")