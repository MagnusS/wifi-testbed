'''
Copyright (c) 2013-2014, Magnus Skjegstad (magnus@skjegstad.com) / FFI
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
'''

import paramiko
from subprocess import Popen, PIPE

class Executor(object):
    """ The executor class is used to execute commands either locally or remotely through an ssh connection."""
    ssh = None # Must be an SSHClient object from Paramiko. None==Execute locally. See enable_ssh_exec()

    def __init__(self, sshclient=None):
        """ Initialize the class. Sshclient can be set to a paramiko.SSHClient() object to execute commands remotely. If sshclient=none, commands are executed locally. """
        self.ssh = sshclient

    def set_ssh_client(self, ssh):
        """ Set remote SSH connection that will be used by execute_cmd(). Set to None to disable ssh. When ssh is disabled, commands are executed locally. """
        self.ssh = ssh

    def execute_cmd(self, command, exception_on_error=True):
        """ Execute command and return output as string. The command will be executed remotely if sshclient is set, otherwise locally. """

        if self.ssh == None:
            p = Popen(command, stdout=PIPE)
            
            if p.wait() == 1:   # Command returned an error code
                if exception_on_error == True:
                    raise Exception("Unable to executed command '%s':\r\n%s" % (command, p.stdout.read()))
                else:
                    return None
            else:
                return p.stdout.read().strip('\n')
        else: # use SSH and execute remotely
            return self.execute_cmd_ssh(command, exception_on_error)

    def execute_cmd_ssh(self, command, exception_on_error=True):
        """ Execute command over SSH and return output as string. Fails if ssh client is not set. """
        try:
            stdin, stdout, stderr = self.ssh.exec_command(" ".join(command))
        except:
            if exception_on_error:
                raise
            else:
                return None
        
        return stdout.read().strip('\n')



