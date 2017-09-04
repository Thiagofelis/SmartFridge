from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^configura/$', views.configura, name='configura'),
    url(r'^$', views.index, name='index'),
]