/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kmjobmanager.h"
#include "kmjob.h"
#include "kmthreadjob.h"
#include "kmfactory.h"

#include <tdeaction.h>
#include <kdebug.h>
#include <tdeconfig.h>

KMJobManager::KMJobManager(TQObject *parent, const char *name)
: TQObject(parent,name)
{
	m_jobs.setAutoDelete(true);
	m_threadjob = new KMThreadJob(this, "ThreadJob");
	m_filter.setAutoDelete(true);
}

KMJobManager::~KMJobManager()
{
}

KMJobManager* KMJobManager::self()
{
	return KMFactory::self()->jobManager();
}

void KMJobManager::discardAllJobs()
{
	TQPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		it.current()->setDiscarded(true);
}

void KMJobManager::removeDiscardedJobs()
{
	for (uint i=0;i<m_jobs.count();i++)
		if (m_jobs.at(i)->isDiscarded())
		{
			m_jobs.remove(i);
			i--;
		}
}

/*KMJob* KMJobManager::findJob(int ID)
{
	TQPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		if (it.current()->id() == ID)
			return it.current();
	return 0;
}*/

KMJob* KMJobManager::findJob(const TQString& uri)
{
	TQPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
		if (it.current()->uri() == uri)
			return it.current();
	return 0;
}

void KMJobManager::addJob(KMJob *job)
{
	// only keep it if "printer" is not empty, and in printer filter
	if (!job->uri().isEmpty() && !job->printer().isEmpty())
	{
		KMJob	*aJob = findJob(job->uri());
		if (aJob)
		{
			aJob->copy(*job);
			delete job;
		}
		else
		{
			job->setDiscarded(false);
			m_jobs.append(job);
		}
	}
	else
		delete job;
}

/*bool KMJobManager::sendCommand(int ID, int action, const TQString& arg)
{
	KMJob	*job = findJob(ID);
	if (job)
	{
		TQPtrList<KMJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}*/

bool KMJobManager::sendCommand(const TQString& uri, int action, const TQString& arg)
{
	KMJob	*job = findJob(uri);
	if (job)
	{
		TQPtrList<KMJob>	l;
		l.setAutoDelete(false);
		l.append(job);
		return sendCommand(l,action,arg);
	}
	return false;
}

bool KMJobManager::sendCommand(const TQPtrList<KMJob>& jobs, int action, const TQString& args)
{
	// split jobs in 2 classes
	TQPtrList<KMJob>	csystem, cthread;
	csystem.setAutoDelete(false);
	cthread.setAutoDelete(false);
	TQPtrListIterator<KMJob>	it(jobs);
	for (;it.current();++it)
		if (it.current()->type() == KMJob::Threaded) cthread.append(it.current());
		else csystem.append(it.current());

	// perform operation on both classes
	if (cthread.count() > 0 && !sendCommandThreadJob(cthread, action, args))
		return false;
	if (csystem.count() > 0 && !sendCommandSystemJob(csystem, action, args))
		return false;
	return true;
}

bool KMJobManager::sendCommandSystemJob(const TQPtrList<KMJob>&, int, const TQString&)
{
	return false;
}

bool KMJobManager::sendCommandThreadJob(const TQPtrList<KMJob>& jobs, int action, const TQString&)
{
	if (action != KMJob::Remove)
		return false;

	TQPtrListIterator<KMJob>	it(jobs);
	bool	result(true);
	for (;it.current() && result; ++it)
		result = m_threadjob->removeJob(it.current()->id());
	return result;
}

bool KMJobManager::listJobs(const TQString&, KMJobManager::JobType, int)
{
	return true;
}

const TQPtrList<KMJob>& KMJobManager::jobList(bool reload)
{
	if (reload || m_jobs.count() == 0)
	{
		discardAllJobs();
		TQDictIterator<JobFilter>	it(m_filter);
		int	joblimit = limit();
		bool threadjobs_updated = false;
		for (; it.current(); ++it)
		{
			if ( it.current()->m_isspecial )
			{
				if ( !threadjobs_updated )
				{
					threadJob()->updateManager( this );
					threadjobs_updated = true;
				}
			}
			else
			{
				if (it.current()->m_type[ActiveJobs] > 0)
					listJobs(it.currentKey(), ActiveJobs, joblimit);
				if (it.current()->m_type[CompletedJobs] > 0)
					listJobs(it.currentKey(), CompletedJobs, joblimit);
			}
		}
		m_threadjob->updateManager(this);
		removeDiscardedJobs();
	}
	return m_jobs;
}

int KMJobManager::actions()
{
	return 0;
}

TQValueList<TDEAction*> KMJobManager::createPluginActions(TDEActionCollection*)
{
	return TQValueList<TDEAction*>();
}

void KMJobManager::validatePluginActions(TDEActionCollection*, const TQPtrList<KMJob>&)
{
}

void KMJobManager::addPrinter(const TQString& pr, KMJobManager::JobType type, bool isSpecial)
{
	struct JobFilter	*jf = m_filter.find(pr);
	if (!jf)
	{
		jf = new JobFilter;
		m_filter.insert(pr, jf);
	}
	jf->m_type[type]++;
	jf->m_isspecial = isSpecial;
}

void KMJobManager::removePrinter(const TQString& pr, KMJobManager::JobType type)
{
	struct JobFilter	*jf = m_filter.find(pr);
	if (jf)
	{
		jf->m_type[type] = TQMAX(0, jf->m_type[type]-1);
		if (!jf->m_type[0] && !jf->m_type[1])
			m_filter.remove(pr);
	}
}

bool KMJobManager::doPluginAction(int, const TQPtrList<KMJob>&)
{
	return true;
}

void KMJobManager::setLimit(int val)
{
	TDEConfig *conf = KMFactory::self()->printConfig();
	conf->setGroup("Jobs");
	conf->writeEntry("Limit", val);
}

int KMJobManager::limit()
{
	TDEConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("Jobs");
	return conf->readNumEntry("Limit", 0);
}

#include "kmjobmanager.moc"
