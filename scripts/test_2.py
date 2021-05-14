import sys
import signal
import datetime
TIMEOUT = 1 # seconds
signal.signal(signal.SIGALRM, input)
signal.alarm(TIMEOUT)



datetime.datetime.now()
try:
    for line in sys.stdin:
        args = line.split("  ")
        nombre = args[0].split("=")
        apellido = args[1].split("=")
        print("Tu nombre es: " + nombre[1] )
        print( "Tu apellido es:" + apellido[1])
        print("\n\n")
        print("La fecha y hora actuales son:")
        print( datetime.datetime.now())
except:
    ignorar = True




