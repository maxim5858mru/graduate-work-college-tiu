from rest_framework import serializers
from database.models import Users, Logs

class UsersDetailSerializer(serializers.ModelSerializer):
    class Meta:
        model = Users
        fields = '__all__'

class LogsDetailSerializer(serializers.ModelSerializer):
    class Meta:
        model = Logs
        fields = '__all__'
