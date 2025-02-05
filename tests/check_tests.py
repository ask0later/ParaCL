from subprocess import run, Popen, PIPE
from sys import executable

num_test = 1
is_ok = True
for i in range(1, 8):
    str_data =  "tests/right/" + str(i) + ".paracl"
    file_in = open(str_data, "r")
    str_ans = "tests/right/" + str(i) + ".ans"

    ans = []
    for i in open(str_ans):
        ans.append(float(i.strip()))
	
    result = run(["build/src/Interpretator"], capture_output = True, encoding='cp866', stdin=file_in)
    print("Test: " + str(num_test).strip())

    res = list(map(float, result.stdout.split()))
    eps = 0.00001

    if len(res) == len(ans):
        fl = True
        for i in range(len(res)):
            if abs(res[i] - ans[i]) > eps:
                fl = False

        if fl:
            print("OK")
        else:
            is_ok = False
            print("ERROR\nExpect:", ans, "\nGive:  ", res)
        
    else:
        print("ERROR\nExpect:", ans, "\nGive:  ", res)
        is_ok = False
        
    print("-------------------------------------------------")
    num_test += 1

if is_ok:
	print("TESTS PASSED")
else:
	print("TESTS FAILED")