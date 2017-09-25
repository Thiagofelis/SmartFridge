from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^resposta_configuracao/$', views.resposta_configuracao, name='resposta_configuracao'),
    url(r'^$', views.index, name='index'),
]