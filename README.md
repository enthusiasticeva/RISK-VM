# RISK_VM
This was an assignment completed in my fourth year of university for COMP2017 - Systems Programming.

The assignment was, given the instructions and structure outlined in [the spec](/a2_spec.pdf), so create a VM that could take in assembly code written in binary, translate and execute it.

To run the VM, run the following 3 commands
```
$ make clean
$ make
$ ./vm_riskxvii <memory_image_binary>
```

The possible `memory_image_binary`, `.mi` files are in the [examples](/examples/) folder.

Note that the `add_2_numbers`, `5_sum`, and `shift` examples take in numbers as input.

No tests were written for this assignment. 