# Get Started

1. [Install gz-acropolis](https://gazebosim.org/docs/acropolis/install).

2. Compile and install gz-launch from source. You will have to `sudo make
   install` so that `gz-tools` find the launch.yaml files.

```
sudo apt-get install libwebsockets-dev
mkdir build
cd build
cmake ../
make
sudo make install
```

3. Run gz-sim.

```
gz sim -v 4
```

4. Run the websocket server. The follow assumes you are in the root of the
   source tree.

```
gz launch -f examples/websocket.gzlaunch -v 4
```

5. Open the `plugins/websocket_server/index.html` in a web browser.

```
firefox plugins/websocket_server/index.html
```


# Authorization

The `websocket_server` plugin accepts to authentication keys:

* `<authorization_key>` : If this is set, then a connection must provide the
matching key using an "auth" call on the websocket. If the `<admin_authorization_key>` is set, then the connection can provide that key.

* `<admin_authorization_key>` : If this is set, then a connection must provide the matching key using an "auth" call on the websocket. If the `<authorization_key>` is set, then the connection can provide that key.

Two keys are used in order to support authorization of different users.
A competition scenario may require admin access while prohibiting user
access.

# SSL

1. Use the `localhost.cert` and `localhost.key` files for testing purposes.
Configure the websocket plugin using:

```
  <ssl>
    <cert_file>PATH_TO_localhost.cert</cert_file>
    <private_key_file>PATH_TO_localhost.key</private_key_file>
  </ssl>
```

   * You can create your own self-signed certificates using the following
   command. Use "localhost" for the  "Common Name" question.

   ```
   openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server.key -out server.cert
   ```

2. Run the plugin.

3. Run a browser, such as firefox, with the `index.html` file.

```
firefox index.html
```

4. Open another browser tab, and go to `https://localhost:9002`. Accept the
   certificate.

5. Refresh the `index.html` browser tab.
