#!/bin/sh

make &&
cd data && sudo make install && cd .. &&
ELM_THEME=gry ELM_SCALE=2 ELM_FINGER_SIZE=80 src/mokojeweled
