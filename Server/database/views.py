from django.shortcuts import render
from rest_framework import generics
from database.serializers import  UsersDetailSerializer, LogsDetailSerializer #, DatabaseListSerializer
from database.models import Users, Logs

# Create your views here.

class UsersCreateView(generics.CreateAPIView):
    serializer_class = UsersDetailSerializer

class UsersDetailView(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = UsersDetailSerializer
    queryset = Users.objects.all()

class LogsCreateView(generics.CreateAPIView):
    serializer_class = LogsDetailSerializer

class LogsDetailView(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = LogsDetailSerializer
    queryset = Logs.objects.all()
