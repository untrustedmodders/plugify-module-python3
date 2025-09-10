#!/bin/bash
# build.sh - For Linux builds

set -ex

# Create the target directories
mkdir -p $PREFIX/bin
mkdir -p $PREFIX/lib
mkdir -p $PREFIX/python3.12
mkdir -p $PREFIX

# Copy the shared library and module file
cp bin/libplugify-module-python3.so $PREFIX/bin/
cp -r lib/* $PREFIX/lib/
cp -r python3.12/* $PREFIX/python3.12/
cp plugify-module-python3.pmodule $PREFIX/

# Set proper permissions
chmod 755 $PREFIX/bin/libplugify-module-python3.so
chmod -R 755 $PREFIX/lib
chmod -R 755 $PREFIX/python3.12
chmod 644 $PREFIX/plugify-module-python3.pmodule

# Create activation scripts for proper library path
mkdir -p $PREFIX/etc/conda/activate.d
mkdir -p $PREFIX/etc/conda/deactivate.d

cat > $PREFIX/etc/conda/activate.d/plugify-module-python3.sh << EOF
#!/bin/bash
export PLUGIFY_PY3_MODULE_PATH="\${CONDA_PREFIX}:\${PLUGIFY_PY3_MODULE_PATH}"
EOF

cat > $PREFIX/etc/conda/deactivate.d/plugify-module-python3.sh << EOF
#!/bin/bash
export PLUGIFY_PY3_MODULE_PATH="\${PLUGIFY_PY3_MODULE_PATH//\${CONDA_PREFIX}:/}"
EOF

chmod +x $PREFIX/etc/conda/activate.d/plugify-module-python3.sh
chmod +x $PREFIX/etc/conda/deactivate.d/plugify-module-python3.sh