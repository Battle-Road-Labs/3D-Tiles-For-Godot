class ourSpawn:
    import subprocess
    def ourspawn(self, sh, escape, cmd, args, env):
        newargs = ' '.join(args[1:])
        cmdline = cmd + " " + newargs
        startupinfo = self.subprocess.STARTUPINFO()
        startupinfo.dwFlags |= self.subprocess.STARTF_USESHOWWINDOW
        proc = self.subprocess.Popen(cmdline, stdin=self.subprocess.PIPE, stdout=self.subprocess.PIPE,
            stderr=self.subprocess.PIPE, startupinfo=startupinfo, shell = False, env = env)
        data, err = proc.communicate()
        rv = proc.wait()
        if rv:
            print ("=====")
            print (err)
            print ("=====")
        return rv

def SetupSpawn( env ):
    import sys
    if sys.platform == 'win32':
        buf = ourSpawn()
        buf.ourenv = env
        env['SPAWN'] = buf.ourspawn