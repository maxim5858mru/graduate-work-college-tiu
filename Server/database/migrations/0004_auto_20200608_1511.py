# Generated by Django 3.0.7 on 2020-06-08 10:11

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('database', '0003_auto_20200608_1418'),
    ]

    operations = [
        migrations.AlterField(
            model_name='database',
            name='door',
            field=models.IntegerField(default=-1, verbose_name='Door'),
        ),
        migrations.AlterField(
            model_name='database',
            name='fingerprintID',
            field=models.IntegerField(default=-1, null=True, verbose_name='Fingerprint ID'),
        ),
        migrations.AlterField(
            model_name='database',
            name='pin',
            field=models.IntegerField(default=-1, verbose_name='PIN'),
        ),
    ]
