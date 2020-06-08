from django.shortcuts import render
from rest_framework import generics
from database.serializers import DatabaseDetailSerializer, LogsDetailSerializer #, DatabaseListSerializer
from database.models import Database, Logs

# Create your views here.

class DatabaseCreateView(generics.CreateAPIView):
    serializer_class = DatabaseDetailSerializer

class DatabaseDetailView(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = DatabaseDetailSerializer
    queryset = Database.objects.all()

class LogsCreateView(generics.CreateAPIView):
    serializer_class = LogsDetailSerializer

class LogsDetailView(generics.RetrieveUpdateDestroyAPIView):
    serializer_class = LogsDetailSerializer
    queryset = Logs.objects.all()
