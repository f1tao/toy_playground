step 1, compile and install the module:

```bash
apxs -c -i -a mod_apache_module_demo.c
```

step 2, add the content of `add_to_http_conf.txt` to the `httpd.conf`.

step 3, restart the `apache` service, then you can play with the toy.