from os import environ

first_stack_ip = environ.get("ACCESS_FIRST_STACK_IP", "192.168.79.16")
first_stack_mask = environ.get("ACCESS_FIRST_STACK_MASK", "255.255.255.0")
first_stack_server_ip = environ.get("ACCESS_FIRST_STACK_SERVER_IP", "192.168.79.10")