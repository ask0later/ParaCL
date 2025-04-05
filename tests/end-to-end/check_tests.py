import sys
from subprocess import run, Popen, PIPE
from sys import executable

generator = sys.argv[1]
num_test = 1
is_ok = True
for i in range(1, 24):
    print("Right tests:")
    str_data =  "right/" + str(i) + ".paracl"
    str_ans = "right/" + str(i) + ".ans"

    ans = []
    for i in open(str_ans):
        ans.append(float(i.strip()))
	
    result = run([generator, str_data], capture_output = True, encoding='cp866')
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

print()
print("==================================================================================================")
print("==================================================================================================")
print("==================================================================================================")
print()
for i in range(1, 11):
    print("Wrong tests:")
    str_data =  "wrong/" + str(i) + ".paracl"
    str_ans = "wrong/" + str(i) + ".ans"

    ans = []
    for i in open(str_ans):
        ans.append(i)
	
    result = run([generator, str_data], capture_output = True, encoding='cp866')
    print("Test: " + str(num_test).strip())

    res = list(result.stdout.split('\n'))
    res1 = []
    for i in range(len(res)):
        if res[i] != '':
            res1.append(res[i])

    ans1 = []
    for i in range(len(ans)):
        if ans[i] != '':
            ans1.append(ans[i][:-1])

    if len(res1) == len(ans1):
        fl = True
        for i in range(len(res1)):
            if res1[i] != ans1[i]:
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
    sys.exit(1)