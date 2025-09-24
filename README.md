# clrgir

## Decription

Checks for git repositories in given path and remove thoses which haven't been edited for X amount of time!

## Install

```
git clone git@github.com:LukasDaxecker/clrgir.git
cd clrgir
chmod +x install.sh
./install.sh
cd ..
rm -rf clrgir
```
## Help

clrdir [-R] [-I] [-T <TIME>] [-P <PATH>]

* -R ... Recursive
* -I ... Ignore uncommitted changes
* -T \<TIME\> ... Time since last edit in days (default 20)
* -P \<PATH\> ... Path to repos (default .)
