## Attach to docker container running on remote server via vscode

Firstly, make sure your ssh user can run docker commands on the remote without sudo. For example, the default user for Ubuntu-based lightsail instances is "ubuntu".

On the remote instance, as user ubuntu:
`sudo gpasswd -a $USER docker`
`newgrp docker`

Now try something like `docker ps -a` without sudo.

## Add ssh key to ssh agent

See for more info: https://code.visualstudio.com/docs/containers/ssh

Add your keyfile to your ssh agent.
`ssh-add /path/to/your/keyfile.pem`

You can erify that your identity is available to the agent with `ssh-add -l`. It should list one or more identities that look something like `2048 SHA256:abcdefghijk somethingsomething (RSA)`.

## Create docker context

Create a docker context via ssh
`docker context create my-remote-docker-machine --docker "host=ssh://username@host:port"`

For lightsail this might look like (if you omit the port, it will default to 22, which is probably fine):
`docker context create friendly-name-of-my-instance --docker "host=ssh://ubuntu@<lightsailipaddress>"`

## Create docker context

Open a new window in vscode, open command pallet with `cmd+shift+p`, type docker contexts: use, hit enter and select `friendly-name-of-my-instance`.

Now open a terminal in vscode and try a docker command like `docker ps -a`. You should see the docker containers available on the remote instance.

## Add remote instance to vscode hosts

In a terminal, try a complete ssh command like `ssh -i /path/to/your/keyfile.pem ubuntu@<remote instance ip>` and make sure you can connect.

Make sure the remote - ssh extension is installed and click on the >< icon in lower-left corner of vscode, select "Connect to host" and "Add new ssh host"

Add the validated ssh command and hit enter.

Now, when you click on the >< icon in lower-left corner of vscode and select "Connect to host" you should see the `<remote instance ip>` you just added.

## Use Remote SSH extension in vscode to open tunnel and attach to docker container

Click on the >< icon in lower-left corner of vscode and select the remote instance ip (you may need to manually click "continue" at the top of the window).
Open a terminal in vscode, your shell should be inside the remote instance.
Make sure `docker ps -a` shows that your docker container is currently running, create or start it if needed.
`cd /home/vaultdb && ls` to arrive at the entrypoint for the zksql repo.
Open command pallette `cmd+shift+p`, type "attach to running container" hit enter, select the container. Vscode should open inside the container. If you don't see "attach to running container" you may need to make sure you have Microsoft's "dev containers" and/or "remote - tunnels" vscode extensions installed.
Hit `cmd+shift+e` to make sure vscode's file explorer is visible, select "open folder", type `/home/vaultdb` and hit enter.
Now both vscode's terminal and file system should now be attached to /vaultdb inside the running docker container on the lightsail instance 🎉.
Note: vscode may leave a extra blank window or two open. You can close these.

## troubleshooting

Got this error when building test suite:

```bash
#fatal: unsafe repository ('/home/vaultdb/zksql/src/main/cpp/_deps/googletest-src' is owned by someone else)
#To add an exception for this directory, call:
#        git config --global --add safe.directory /home/vaultdb/zksql/src/main/cpp/_deps/googletest-src
```
