# -*- coding: utf-8 -*-

import socket
import sys
import json

def prettyPrint(d, i=0):
    def spaces(n_spaces):
        for i in range(0, 2*n_spaces):
            sys.stdout.write(' ')

    def prettyPrintKeyValue(key, value):
        spaces(i)
        sys.stdout.write('{}'.format(key))
        if isinstance(value, (dict,list)):
            sys.stdout.write(':\n')
            prettyPrint(value, i + 1)
        else:
            print(' = {}'.format(value))

    if isinstance(d, dict):
        for k in d:
            prettyPrintKeyValue(k, d[k])
    elif isinstance(d, list):
        pos = 0
        for v in d:
            prettyPrintKeyValue(pos, v)
            pos += 1
    else:
        spaces(i)
        print(d)

def show(resp):
    if not isinstance(resp, dict):
        print(resp)
    elif 'error' in resp:
        print('Error: {}'.format(resp['error']))
    elif 'options' in resp:
        if isinstance(resp['options'], list):
            print('Attributes:')
            for opt in resp['options']:
                print('  - {}: {}'.format(opt['attribute'], opt['type']))
            if 'values' in resp:
                print('Values:')
                prettyPrint(resp['values'], 1)
        else:
            print('Array with {} elements'.format(resp['options']['elements']))
    else:
        prettyPrint(resp)

def ppp(s):
    try:
        return json.loads(s)
    except:
        return s

sock = socket.socket(socket.AF_INET6,
                     socket.SOCK_STREAM,
                     socket.IPPROTO_TCP)
sock.settimeout(1)
try:
    sock.connect(('::1', 32145))
except:
    print("Cannot connect to the game")
else:
    args = map(lambda x: x.split('='), sys.argv[1:])
    args = map(lambda s: [ s[0], s[1] if len(s) != 1 else None], args)
    args = map(lambda s: { 'command': s[0], 'value': ppp(s[1]) }, args)
    args = list(args)
    sock.send(json.dumps(args).encode('utf-8'))
    try:
        data = sock.recv(1500)
        resp = json.loads(data.decode('utf-8'))
        #print(data.decode('utf-8'))
        for r in resp: show(r)
    except socket.timeout:
        print("Did not receive anything from game, giving up")
    sock.close()
