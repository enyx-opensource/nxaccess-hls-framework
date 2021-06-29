from .ip_from_environment import first_stack_ip, first_stack_mask
from enyx_oe import TCPStackIPConfiguration, IPv4Address

def setup_first_tcp_ip_stack(board):
    stack = TCPStackIPConfiguration(board, 0)
    ip = IPv4Address(first_stack_ip)
    netmask = IPv4Address(first_stack_mask)
    return stack.configure(ip, netmask)