#!/bin/sh

ps aux | grep CellModel | awk '{print $2}' | xargs -r kill -9
