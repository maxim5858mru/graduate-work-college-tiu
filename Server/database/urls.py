from django.contrib import admin
from django.urls import path, include
from database.views import *


app_name = 'database'
urlpatterns = [
    path('users/create', UsersCreateView.as_view()),
    path('users/detail/<int:pk>', UsersDetailView.as_view()),
    path('logs/create', LogsCreateView.as_view()),
    path('logs/detail/<int:pk>', LogsDetailView.as_view())
]
