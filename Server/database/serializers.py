from rest_framework import serializers
from database.models import Database, Logs

class DatabaseDetailSerializer(serializers.ModelSerializer):
    class Meta:
        model = Database
        fields = '__all__'

class LogsDetailSerializer(serializers.ModelSerializer):
    class Meta:
        model = Logs
        fields = '__all__'
