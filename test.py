import json
import os
import subprocess
import sys


subprocess.run("make")
test_cases_amn = int(len(os.listdir("testing/files")) / 2)

if "--create" in sys.argv:
    open("testing/files/" + str(test_cases_amn) + "_code.txt", 'w')
    with open("testing/files/" + str(test_cases_amn) + "_result.json", 'w') as file:
        file.write('{\n\t"output": "",\n\t"return_code": "",\n\t"description": ""\n}')
    sys.exit(0)

for i in range(min(test_cases_amn, 2000)):
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

if "--memory" in sys.argv:
    print("\nNow checking for memory leaks.")
    for i in range(min(test_cases_amn, 2000)):
        process = subprocess.run(f"valgrind ./parse testing/files/{i}_code.txt", shell=True, capture_output=True, text=True)
        output = process.stderr

        print(f"Checking {i}")
        assert "All heap blocks were freed -- no leaks are possible" in output, f"Memory leak in test case {i}."

    print(f"No memory leaks! {test_cases_amn} total test cases.")