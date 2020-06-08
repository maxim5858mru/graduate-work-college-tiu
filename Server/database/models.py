from django.db import models
from django.contrib.postgres.fields import ArrayField
from django.contrib.auth import get_user_model
User = get_user_model()

# Create your models here.
# После изменения ./manage.py makemigrations & python manage.py migrate
# Отключение сервера sudo fuser -k 8000/tcp

class Database(models.Model):
    firstName = models.CharField(verbose_name='First Name', max_length=10, default="Unset")
    lastName = models.CharField(verbose_name='Last Name', max_length=10, default="Unset")
    middleName = models.CharField(verbose_name='Middle Name', max_length=10, default="Unset")
    door = models.IntegerField(verbose_name='Door', default=-1)
    methodPIN = models.BooleanField(verbose_name='Method PIN', default=False)
    methodRFID = models.BooleanField(verbose_name='Method RFID', default=False)
    methodFPID = models.BooleanField(verbose_name='Method FP', default=False)
    pin = models.IntegerField(verbose_name='PIN', default=-1)
    fingerprintID = models.IntegerField(verbose_name='Fingerprint ID', null=True, default=-1)
    rfid1 = models.IntegerField(verbose_name='RFID 1', default=0)
    rfid2 = models.IntegerField(verbose_name='RFID 2', default=0)
    rfid3 = models.IntegerField(verbose_name='RFID 3', default=0)
    rfid4 = models.IntegerField(verbose_name='RFID 4', default=0)
    rfid5 = models.IntegerField(verbose_name='RFID 5', default=0)
    rfid6 = models.IntegerField(verbose_name='RFID 6', default=0)
    rfid7 = models.IntegerField(verbose_name='RFID 7', default=0)
    rfid8 = models.IntegerField(verbose_name='RFID 8', default=0)
    rfid9 = models.IntegerField(verbose_name='RFID 9', default=0)
    rfid10 = models.IntegerField(verbose_name='RFID 10', default=0)
    startTime = models.IntegerField(verbose_name='Start Time', null=True)
    endTime = models.IntegerField(verbose_name='End Time', null=True)
    user = models.ForeignKey(User, verbose_name='User', on_delete=models.CASCADE, default=1)
    objects = models.Manager()

class Logs(models.Model):
    LOG_TYPE = (
        (0, 'SUCCESSFUL_AUTH'),
        (1, 'OPEN'),
        (2, 'FALL_FIRST_AUTH'),
        (3, 'FALL_SEC_AUTH'),
    )
    type = models.IntegerField(verbose_name='Type of log entry', choices=LOG_TYPE)
    seconds = models.IntegerField(verbose_name='Seconds', null=True)
    minutes = models.IntegerField(verbose_name='Minutes', null=True)
    day = models.IntegerField(verbose_name='Day', null=True)
    month = models.IntegerField(verbose_name='Month', null=True)
    year = models.IntegerField(verbose_name='Year', null=True)
    door = models.IntegerField(verbose_name='Door', default=-1)
    userID = models.IntegerField(verbose_name='User ID', default=-1)
    user = models.ForeignKey(User, verbose_name='User', on_delete=models.CASCADE, default=1)
    objects = models.Manager()
