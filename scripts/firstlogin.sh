#!/usr/bin/env bash
## each time you make a completely new docker machine this will happen...

###this kinda needs to be parametrized, 

ssh-keygen -f "/home/frederico/.ssh/known_hosts" -R tsn_caffe
ssh-keygen -f "/home/frederico/.ssh/known_hosts" -R 172.28.5.3

ssh -oHostKeyAlgorithms='ssh-rsa' root@tsn_caffe -p 22
