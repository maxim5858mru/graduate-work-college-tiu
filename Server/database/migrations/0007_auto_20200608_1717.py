# Generated by Django 3.0.7 on 2020-06-08 12:17

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('database', '0006_logs_user'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='logs',
            name='time',
        ),
        migrations.AddField(
            model_name='logs',
            name='minutes',
            field=models.IntegerField(null=True, verbose_name='Minutes'),
        ),
        migrations.AddField(
            model_name='logs',
            name='seconds',
            field=models.IntegerField(null=True, verbose_name='Seconds'),
        ),
    ]