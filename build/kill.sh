kill -9 `ps aux | grep testauto.run | head -n 1 | sed -n 's/  */ /gp' | cut -f 2 -d \  `
